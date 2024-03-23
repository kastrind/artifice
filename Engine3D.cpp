#include "Engine3D.h"

Engine3D::Engine3D(int width, int height,
				   float near, float far,
				   float fov, float dof,
				   float collidingDistance,
				   float gravitationalPull,
				   float cameraSpeedFactor,
				   UserMode userMode, EventController* ec)
				   : width(width), height(height),
				   near(near), far(far),
				   fov(fov), dof(dof),
				   collidingDistance(collidingDistance),
				   gravitationalPull(gravitationalPull),
				   cameraSpeedFactor(cameraSpeedFactor),
				   userMode(userMode), eventController(ec)
{
	isActive = false;

	isTouched = false;

	updateVerticesFlag = false;

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);

	//camera
	cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	if (userMode == UserMode::EDITOR)
	{
		this->gravitationalPull = 0.0f;
		originalCollidingDistance = this->collidingDistance;
		this->collidingDistance = -0.1f;
	}
}

std::thread Engine3D::startEngine()
{
	//start thread
	std::thread t = std::thread(&Engine3D::engineThread, this);

	return t;
}

std::thread Engine3D::listenForInput()
{
	//start thread
	std::thread t = std::thread(&Engine3D::inputListenerThread, this);

	return t;
}

void Engine3D::inputListenerThread()
{
	while(!isActive) {}
	while (isActive)
	{
		keysPressed = eventController->getKeysPressed();
		if (keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]) std::cout << "LCLICK!\n";
	}
}

void Engine3D::engineThread()
{
	//create user resources as part of this thread
	if (!onUserCreate())
		isActive = false;
	else
		isActive = true;

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	//run as fast as possible
	while (isActive)
	{
		//handle timing
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTimeDuration = tp2 - tp1;
		tp1 = tp2;
		elapsedTime = elapsedTimeDuration.count();

		//handle frame update
		if (!onUserUpdate(elapsedTime))
			isActive = false;
	}

	if (onUserDestroy())
	{
		//user has permitted destroy, so exit and clean up
		isActive = false;
	}
	else
	{
		//user denied destroy for some reason, so continue running
		isActive = true;
	}
}

bool Engine3D::onUserCreate()
{
	return true;
}

bool Engine3D::onUserUpdate(float elapsedTime)
{
	mtx.lock();

	glm::vec3 prevCameraPos = cameraPos;
	glm::vec3 center{ 0, 0, 0 };
	float modelDistance = dof;
	float maxModelDist = 0.0f;
	glm::vec4 collidingTriPts[3];

	move(elapsedTime);

	if (modelsInFocus.size()) { prevModelInFocus = *(modelsInFocus.begin()); }

	collides = false;
	collidesFront = false;
	collidesBack = false;
	collidesRight = false;
	collidesLeft = false;

	canSlide = false;
	hasLanded = false;

	modelsInFocus.clear();

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);
	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	mtx.unlock();

	//for each model to raster
	for (auto &model : modelsToRaster)
	{

		mtx.lock();
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, model.position);
		model.modelMatrix = modelMatrix;

		model.inFocus = false;
		bool checkAgainForFocus = true;

		mtx.unlock();

		modelDistance = dof;

		for (auto tri : model.modelMesh.tris)
		{
			glm::vec4 pt[3];
			pt[0] = tri.p[0];
			pt[1] = tri.p[1];
			pt[2] = tri.p[2];

			//model transformation
			pt[0] = modelMatrix * pt[0];
			pt[1] = modelMatrix * pt[1];
			pt[2] = modelMatrix * pt[2];

			//finds distance from model
			glm::vec4 avgP = ( (pt[0]) + (pt[1]) + (pt[2]) ) / 3.0f;
			float avgDist = glm::distance(cameraPos, glm::vec3(avgP));
			model.distance = avgDist;
			float dist1 = glm::distance(cameraPos, glm::vec3(pt[0]));
			float dist2 = glm::distance(cameraPos, glm::vec3(pt[1]));
			float dist3 = glm::distance(cameraPos, glm::vec3(pt[2]));
			float maxDist = std::max(dist1, std::max(dist2, dist3));
			//std::cout << "maxDist: " << maxDist << std::endl;
			if (avgDist < modelDistance) {
				modelDistance =  avgDist;
				maxModelDist = maxDist;
				collidingTriPts[0] = pt[0];
				collidingTriPts[1] = pt[1];
				collidingTriPts[2] = pt[2];
			}
			//std::cout << "modelDistance: " << modelDistance << std::endl;

			//camera/view transformation
			pt[0] = projectionMatrix * viewMatrix * pt[0];
			pt[1] = projectionMatrix * viewMatrix * pt[1];
			pt[2] = projectionMatrix * viewMatrix * pt[2];

			mtx.lock();
			tri.p[0] = pt[0];
			tri.p[1] = pt[1];
			tri.p[2] = pt[2];
			mtx.unlock();

			//determines whether looking at the current model
			if (checkAgainForFocus && tri.contains(glm::vec4(center, 1.0f)))
			{
				model.inFocus = true;
				modelsInFocus.insert(&model);
				checkAgainForFocus = false;
			}

		}

		//marks out-of-range models
		model.isInDOF = modelDistance < dof;

		//detect if colliding with the model
		if (model.isSolid && modelDistance <= collidingDistance * 1.5f) {
			//std::cout << "modelDistance: " << modelDistance << std::endl;

			//get the triangle normal
			glm::vec3 line1 = collidingTriPts[1] - collidingTriPts[0];
			glm::vec3 line2 = collidingTriPts[2] - collidingTriPts[0];
			glm::vec3 normal = glm::normalize(glm::cross(line1, line2));
			// std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

			//based on dp and normal, determine if able to slide and the desired motion
			float dpFront = glm::dot(cameraFront, normal);
			float absDP = std::abs(dpFront);
			float dpRight = glm::dot(cameraRight, normal);
			float dpLeft = glm::dot(-cameraRight, normal);
			float dpBack = glm::dot(-cameraFront, normal);

			//normal is up means collision with floor
			if (collidingTriPts[0].y <= cameraPos.y && normal.y < normal.z && normal.y < normal.x) {
				hasLanded = true;
			//else collision with wall
			}else if (modelDistance < collidingDistance && maxModelDist < collidingDistance * 1.5f) {
				canSlide = absDP < 0.8f && absDP > 0.0f;
				//std::cout << "dpFront: " << dpFront << std::endl;
				//std::cout << "maxModelDist: " << maxModelDist << std::endl;
				//std::cout << "modelDistance: " << modelDistance << std::endl;
				collides = true;
				if (!collidesFront) collidesFront = dpFront < dpRight && dpFront < dpLeft && dpFront < dpBack;
				if (!collidesBack)  collidesBack  = dpBack < dpRight && dpBack < dpLeft && dpBack < dpFront;
				if (!collidesRight) collidesRight = dpRight < dpLeft && dpRight < dpFront && dpRight < dpBack;
				if (!collidesLeft)  collidesLeft  = dpLeft < dpRight && dpLeft < dpFront && dpLeft < dpBack;
				//std::cout << "front: " << collidesFront << ", right: " << collidesRight << ", left: " << collidesLeft << ", back: " << collidesBack << std::endl;
				glm::vec3 undesiredMotion = normal * dpFront;
				desiredMotion = cameraFront - undesiredMotion;
				desiredMotion = glm::normalize(desiredMotion);
				//std::cout << desiredMotion.x << ", " << desiredMotion.z << std::endl;
			}
		}

		if (model.isSolid && modelDistance < collidingDistance * 0.5f)
		{
			cameraPos = prevCameraPos;
		}
	}
	lightPos = cameraFront;

	edit();

	// std::cout << "collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
	return true;
}



void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(cameraSpeedFactor * elapsedTime);

	cameraPos += !hasLanded ? gravitationalPull * cameraSpeed * glm::vec3(0, -1, 0) : glm::vec3(0, 0, 0);

	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = (float)mouseDistanceX * 5;
		float multiplierY = (float)mouseDistanceY * 5;

		//WSAD camera movement here
		if (keysPressed[SupportedKeys::W] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::W] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::W] && (!hasLanded || userMode == UserMode::EDITOR)) {
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos -= 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos -= cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::S] && (!hasLanded || userMode == UserMode::EDITOR)) {
			cameraPos -= cameraSpeed * cameraFront;
		}
		if (keysPressed[SupportedKeys::A] && hasLanded && collides && !collidesLeft) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos += 0.5f * cameraSpeed * desiredMotion;
			else cameraPos -= 0.5f * cameraSpeed * desiredMotion;
		
		}else if (keysPressed[SupportedKeys::A] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		
		} else if (keysPressed[SupportedKeys::D] && hasLanded && collides && !collidesRight) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos -= 0.5f * cameraSpeed * desiredMotion;
			else cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::D] && (hasLanded && !collides || userMode == UserMode::EDITOR)) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		//if in editor mode, arrows control track camera movement to the sides, above or below camera position
		if (userMode == UserMode::EDITOR)
		{
			if (keysPressed[SupportedKeys::LEFT_ARROW]) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			} else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			}
			if (keysPressed[SupportedKeys::UP_ARROW]) {
				cameraPos -= glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			} else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
				cameraPos += glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			}
		}

		//mouse motion
		if (keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw -= multiplierX * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw += multiplierX * elapsedTime;
		}

		if (keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += multiplierY * elapsedTime;
		} else if (keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= multiplierY * elapsedTime;
		}

		//make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//make sure yaw does not become too large
		if (yaw > 360.0f)
			yaw -= 360.0f;
		if (yaw < 0.0f)
			yaw += 360.0f;

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);

		cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
	}
}


void Engine3D::edit()
{
	if (userMode != UserMode::EDITOR) return;

	if (eventController != nullptr)
	{

		bool* keysPressed = eventController->getKeysPressed();

		// pressing LCTRL + mouse wheel up/down increases/decreases the collation height
		if (keysPressed[SupportedKeys::LEFT_CTRL] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && !isTouched) {
			collationHeight = std::max(--collationHeight, (unsigned int)1);
			std::cout << "set collation height: " << collationHeight << std::endl;
			isTouched = true;
		}else if (keysPressed[SupportedKeys::LEFT_CTRL] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] && !isTouched) {
			std::cout << "set collation height: " << ++collationHeight << std::endl;
			isTouched = true;
		}

		if (editingModel == nullptr && eventController->transitionedMouseLeftButton() && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] && !updateVerticesFlag) {
			model mdl; mdl.texture = "box";
			editingWidth = 0.2f; editingHeight = 0.2f; editingDepth = 0.2f;
			cube cube0{editingWidth};
			cube0.toTriangles(mdl.modelMesh.tris);
			mdl.modelMesh.shape = Shape::CUBE;
			mdl.position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
			mdl.isEditing = true;
			modelsToRaster.push_back(mdl);
			editingModel = &modelsToRaster.back();
			mtx.lock();
			updateVerticesFlag = true;
			mtx.unlock();
			cameraSpeedFactor /= 100;
		}
		
		if (editingModel != nullptr) {
			editingModel->position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
		}

		// releasing left mouse click places a new model
		if (editingModel != nullptr && eventController->transitionedMouseLeftButton() && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]==false && !updateVerticesFlag) {
			model mdl = modelsToRaster.back();
			modelsToRaster.pop_back();
			mtx.lock();
			updateVerticesFlag = true;
			mtx.unlock();
			editingModel = nullptr;
			cameraSpeedFactor *= 100;
			bool isValidPlacement = false;

			// pressing LCTRL enables collation of new model to the previous one along the direction the camera is facing
			if (keysPressed[SupportedKeys::LEFT_CTRL] && modelsInFocus.size() > 0)
			{
				glm::vec3 pos = glm::vec3(0, 0, 0);
				if (std::abs(cameraFront.z) > std::abs(cameraFront.x) && std::abs(cameraFront.z) > std::abs(cameraFront.y))
				{
					if (std::abs(cameraFront.x) > std::abs(cameraFront.y))
						pos.x = cameraFront.x / std::abs(cameraFront.x);
					else
						pos.y = cameraFront.y / std::abs(cameraFront.y);
				}else if (std::abs(cameraFront.x) > std::abs(cameraFront.y) && std::abs(cameraFront.x) > std::abs(cameraFront.z))
				{
					if (std::abs(cameraFront.y) > std::abs(cameraFront.z))
						pos.y = cameraFront.y / std::abs(cameraFront.y);
					else
						pos.z = cameraFront.z / std::abs(cameraFront.z);
				}else if (std::abs(cameraFront.y) > std::abs(cameraFront.x) && std::abs(cameraFront.y) > std::abs(cameraFront.z))
				{
					if (std::abs(cameraFront.x) > std::abs(cameraFront.z))
						pos.x = cameraFront.x / std::abs(cameraFront.x);
					else
						pos.z = cameraFront.z / std::abs(cameraFront.z);
				}
				
				auto modelInFocus = *(modelsInFocus.begin());
				mdl.position = modelInFocus->position + editingDepth * pos;
				isValidPlacement = true;

			// standard placement in front of the camera, no collation
			}else if (!keysPressed[SupportedKeys::LEFT_CTRL])
			{
				mdl.position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
				isValidPlacement = true;
			}

			if (isValidPlacement)
			{
				modelsToRaster.push_back(mdl);

				for (unsigned int i = 1; i < collationHeight; i++) {
					model m = mdl;
					m.position.y += i * editingHeight;
					modelsToRaster.push_back(m);
				}
				mtx.lock();
				updateVerticesFlag = true;
				mtx.unlock();
			}
			//std::cout << "added cube!" << cameraFront.x << cameraFront.y << cameraFront.z << std::endl;

		// right mouse click deletes the model currently in focus
		}else if (eventController->transitionedMouseRightButton() && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] && modelsInFocus.size() > 0 && !updateVerticesFlag) {

			auto modelInFocus = *(modelsInFocus.begin());
			modelInFocus->removeFlag = true;
			modelsToRaster.erase(std::remove_if(modelsToRaster.begin(), modelsToRaster.end(), [](model m) { return m.removeFlag == true; }), modelsToRaster.end());
			mtx.lock();
			updateVerticesFlag = true;
			mtx.unlock();
		}

		/*
		if (modelsInFocus.size())
		{
			if (prevModelInFocus != *(modelsInFocus.begin()))
			{
				auto modelInFocus = *(modelsInFocus.begin());
				for (triangle& tri : modelInFocus->modelMesh.tris) 
				{
					tri.R = 0; tri.G = 192; tri.B = 0;
				}
				if (prevModelInFocus != nullptr)
				{
					for (triangle& tri : prevModelInFocus->modelMesh.tris)
					{
						tri.R = 255; tri.G = 255; tri.B = 255;
					}
				}
				//updateVerticesFlag = true;
			}
		}*/

	}
}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;
	return true;
}

glm::mat4 Engine3D::getProjectionMatrix() const
{
	return projectionMatrix;
}

glm::vec3 Engine3D::getCameraPos() const
{
	return cameraPos;
}

glm::vec3 Engine3D::getCameraFront() const
{
	return cameraFront;
}

glm::vec3 Engine3D::getCameraUp() const
{
	return cameraUp;
}

glm::vec3 Engine3D::getLightPos() const
{
	return lightPos;
}

glm::mat4 Engine3D::getViewMatrix() const
{
	//glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(viewMatrix)); 
	//return viewWithoutTranslation;
	return viewMatrix;
}

void Engine3D::updateVertices()
{
	std::cout << "Updating vertices" << std::endl;
	std::vector<GLfloat> vertexData;
	std::vector<GLuint> indexData;
	GLuint indexCounter = 0;
	std::vector<GLfloat> cubeVertexData;
	std::vector<GLuint> cubeIndexData;
	GLuint cubeIndexCounter = 0;

	std::vector<GLfloat>* vdp = &vertexData;

	//populate vertex vectors with triangle vertex information for each model
	for (auto &model : modelsToRaster)
	{

		vdp = model.modelMesh.shape == Shape::CUBE ? &cubeVertexData : &vertexData;

		for (auto &tri : model.modelMesh.tris)
		{
			glm::vec3 line1 = tri.p[1] - tri.p[0];
			glm::vec3 line2 = tri.p[2] - tri.p[0];
			glm::vec3 normal = glm::normalize(glm::cross(line1, line2));

			for (int i = 0; i < 3; i++)
			{
				vdp->push_back(tri.p[i].x);
				vdp->push_back(tri.p[i].y);
				vdp->push_back(tri.p[i].z);
				vdp->push_back(normal.x);
				vdp->push_back(normal.y);
				vdp->push_back(normal.z);
				vdp->push_back((float)tri.R/255.0f);
				vdp->push_back((float)tri.G/255.0f);
				vdp->push_back((float)tri.B/255.0f);
				vdp->push_back(tri.t[i].x);
				vdp->push_back(tri.t[i].y);
				if (model.modelMesh.shape != Shape::CUBE)
				{
					indexData.push_back(indexCounter++);
				}
				else
				{
					cubeIndexData.push_back(cubeIndexCounter++);
				}
			}
		}
	}

	std::cout << "  vertex data size: " << vertexData.size() << std::endl;
	std::cout << "  index data size: " << indexData.size() << std::endl;
	std::cout << "  cube vertex data size: " << cubeVertexData.size() << std::endl;
	std::cout << "  cube index data size: " << cubeIndexData.size() << std::endl;

	//update VBO
	glBindVertexArray(*gVAO);
	glBindBuffer( GL_ARRAY_BUFFER, *gVBO );
	glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof(GL_FLOAT), vertexData.data(), GL_STATIC_DRAW );

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);

	//update IBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GL_UNSIGNED_INT), indexData.data(), GL_STATIC_DRAW );

	//update cubeVBO
	glBindVertexArray(*gCubeVAO);
	glBindBuffer( GL_ARRAY_BUFFER, *gCubeVBO );
	glBufferData( GL_ARRAY_BUFFER, cubeVertexData.size() * sizeof(GL_FLOAT), cubeVertexData.data(), GL_STATIC_DRAW );

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);
	//normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
	//color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(6 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(2);
	//texture coord attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GL_FLOAT), (void*)(9 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(3);

	//update cubeIBO
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *gCubeIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, cubeIndexData.size() * sizeof(GL_UNSIGNED_INT), cubeIndexData.data(), GL_STATIC_DRAW );

	updateVerticesFlag = false;

	std::cout << "Updated vertices" << std::endl;
}

void Engine3D::render()
{
	//if (updateVerticesFlag) updateVertices();

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cubeMapShader->bind();
	cubeMapShader->setMat4("projection", getProjectionMatrix());
	cubeMapShader->setMat4("view", getViewMatrix());
	//lighting
	cubeMapShader->setVec3("lightPos", getLightPos());
	cubeMapShader->setVec3("viewPos", getCameraPos());
	cubeMapShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	textureShader->bind();
	textureShader->setMat4("projection", getProjectionMatrix());
	textureShader->setMat4("view", getViewMatrix());
	//lighting
	textureShader->setVec3("lightPos", getLightPos());
	textureShader->setVec3("viewPos", getCameraPos());
	textureShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	unsigned int cubeCnt = 0;
	unsigned int prevCubeTrisSize = 0;
	unsigned int cnt = 0;
	for (auto &model : modelsToRaster)
	{

		if (model.modelMesh.shape == Shape::CUBE)
		{
			//ignore out-of-range models
			if (!model.isInDOF) { cubeCnt++; prevCubeTrisSize = model.modelMesh.tris.size(); continue; }

			cubeMapShader->bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *gCubeIBO);
			glBindVertexArray(*gCubeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, (*cubemapIdsMap)[model.texture]);
			cubeMapShader->setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(((cubeCnt++) * (prevCubeTrisSize * 3) ) * sizeof(GL_UNSIGNED_INT)));
			prevCubeTrisSize = model.modelMesh.tris.size();
		}
		else
		{
			//ignore out-of-range models
			if (!model.isInDOF) { cnt += model.modelMesh.tris.size() * 3; continue; }

			textureShader->bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *gIBO);
			glBindVertexArray(*gVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*textureIdsMap)[model.texture]);
			textureShader->setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(( cnt ) * sizeof(GL_UNSIGNED_INT)));
			cnt += model.modelMesh.tris.size() * 3;
		
		}
		glBindVertexArray(0);
	}
}

