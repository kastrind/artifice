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

std::thread Engine3D::startRendering()
{
	//start thread
	std::thread t = std::thread(&Engine3D::renderingLoop, this);

	return t;
}

void Engine3D::renderingLoop()
{
	while(!isActive) {}
	while (isActive)
	{
		render();
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

	captureInput();

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
	for (auto &model : modelsToRender)
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
				mtx.lock();
				modelsInFocus.insert(&model);
				mtx.unlock();
				checkAgainForFocus = false;
			}

		}

		//marks out-of-range models
		model.isInDOF = modelDistance < dof;

		//detect if colliding with the model
		if (model.isSolid && modelDistance <= collidingDistance * 1.5f)
		{
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

	edit(elapsedTime);

	// std::cout << "collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
	return true;
}

void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(cameraSpeedFactor * elapsedTime);

	cameraPos += !hasLanded ? gravitationalPull * cameraSpeed * glm::vec3(0, -1, 0) : glm::vec3(0, 0, 0);

	if (eventController != nullptr)
	{
		//bool* keysPressed = eventController->getKeysPressed();

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


void Engine3D::edit(float elapsedTime)
{
	if (userMode != UserMode::EDITOR) return;

	if (updateVerticesFlag) return;

	if (eventController != nullptr)
	{
		//bool* keysPressed = eventController->getKeysPressed();
		bool isEdited = false;

		// pressing LCTRL + mouse wheel up/down increases/decreases the collation height
		if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationHeight = std::max(--collationHeight, (unsigned int)1);
			std::cout << "collation height: " << collationHeight << std::endl;

		}else if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationHeight++;
			std::cout << "collation height: " << collationHeight << std::endl;
		// pressing LSHIFT + mouse wheel up/down increases/decreases the collation width
		}else if (keysPressed[SupportedKeys::LEFT_SHIFT] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationWidth = std::max(--collationWidth, (unsigned int)1);
			std::cout << "collation width: " << collationWidth << std::endl;
		}else if (keysPressed[SupportedKeys::LEFT_SHIFT] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationWidth++;
			std::cout << "collation width: " << collationWidth << std::endl;
		}else if (prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (++edShapeInt > Shape::CUBE) edShapeInt = 1;
			editingShape = (Shape)edShapeInt;
			std::cout << "shape: " << shapeToString(editingShape) << std::endl;
		}else if (prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (--edShapeInt < 1) edShapeInt = Shape::CUBE;
			editingShape = (Shape)edShapeInt;
			std::cout << "shape: " << shapeToString(editingShape) << std::endl;
		}

		if (editingModel == nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]) {
			model mdl; mdl.texture = "box";
			editingWidth = 0.2f; editingHeight = 0.2f; editingDepth = 0.2f;
			cube cube0{editingWidth};
			cube0.toTriangles(mdl.modelMesh.tris);
			mdl.modelMesh.shape = Shape::CUBE;
			mdl.position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
			mdl.sn = cubePointsCnt;
			cubePointsCnt += mdl.modelMesh.tris.size() * 3;
			std::cout << "about to place model with sn = " << mdl.sn << std::endl;
			mdl.isEditing = true;
			mtx.lock();
			if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
			modelsToRender.push_back(mdl);
			editingModel = &modelsToRender.back();
			std::cout << "placed model has sn = " << editingModel->sn << std::endl;
			mtx.unlock();
			cameraSpeedFactor /= 100;
			isEdited = true;
		}
		
		if (editingModel != nullptr) {
			editingModel->position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
		}

		// releasing left mouse click places a new model
		if (editingModel != nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]==false) {
			cameraSpeedFactor *= 100;
			char heightAlongAxis = 'y';
			char widthAlongAxis = 'x';
			short dirX = cameraFront.x / std::abs(cameraFront.x);
			short dirY = cameraFront.y / std::abs(cameraFront.y);
			short dirZ = cameraFront.z / std::abs(cameraFront.z);

			glm::vec3 pos = glm::vec3(0, 0, 0);
			if (std::abs(cameraFront.z) > std::abs(cameraFront.x)*1.2f && std::abs(cameraFront.z) > std::abs(cameraFront.y)*1.2f)
			{
				heightAlongAxis = 'y';
				widthAlongAxis = 'z';
				pos.z = dirZ;
			} else if (std::abs(cameraFront.x) > std::abs(cameraFront.y)*1.2f && std::abs(cameraFront.x) > std::abs(cameraFront.z)*1.2f)
			{
				heightAlongAxis = 'y';
				widthAlongAxis = 'x';
				pos.x = dirX;
			} else if (std::abs(cameraFront.y) > std::abs(cameraFront.x)*1.2f && std::abs(cameraFront.y) > std::abs(cameraFront.z)*1.2f)
			{
				if (std::abs(cameraFront.x) > std::abs(cameraFront.z)) {
					heightAlongAxis = 'x';
					widthAlongAxis = 'z';
					pos.z = dirZ;
				} else {
					heightAlongAxis = 'z';
					widthAlongAxis = 'x';
					pos.x = dirX;
				}
			}

			model m = *editingModel;
			bool isGlued = false;
			if (keysPressed[SupportedKeys::LEFT_CTRL] && modelInFocusTmp.inFocus) {
				m.position = modelInFocusTmp.position + editingDepth * pos;
				modelInFocusTmp.inFocus = false;
				mtx.lock();
				model* m2 = &modelsToRender.back();
				cubePointsCnt -= m2->sn;
				modelsToRender.pop_back();
				m.sn = cubePointsCnt;
				cubePointsCnt += m.modelMesh.tris.size() * 3;
				modelsToRender.push_back(m);
				mtx.unlock();
				isGlued = true;
				std::cout << "glued placement" << std::endl;
			}else {
				editingModel->position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
				std::cout << "standard placement" << std::endl;
			}

			glm::vec3 initialPos = m.position;
			std::cout << "dir X Y Z : " << dirX << ", " << dirY << ", " << dirZ << std::endl;
			mtx.lock();
			std::cout << "models size before: " << modelsToRender.size() << std::endl;
			for (unsigned int i = 0; i < collationWidth; i++) {
				unsigned int j = i > 0 ? 0 : 1;
				for (j; j < collationHeight; j++) {
					std::cout << "into collation height loop!" << std::endl;
					//std::cout << "repeating for model " << i;
					//m.position.y += editingHeight;
					if (heightAlongAxis == 'y') m.position.y += editingHeight;
					else if (heightAlongAxis == 'x') m.position.x += dirX * editingWidth;
					else if (heightAlongAxis == 'z') m.position.z += dirZ *  editingDepth;
					m.sn = cubePointsCnt;
					cubePointsCnt += m.modelMesh.tris.size() * 3;
					modelsToRender.push_back(m);
				}
				if (widthAlongAxis == 'z') m.position.z += dirZ * editingDepth;
				else if (widthAlongAxis == 'x') m.position.x += dirX * editingWidth;
				else if (widthAlongAxis == 'y') m.position.y += editingHeight;
				//m.position.y = initialPos.y - editingHeight;
				if (heightAlongAxis == 'y') m.position.y = initialPos.y - editingHeight;
				else if (heightAlongAxis == 'x') m.position.x = initialPos.x - dirX * editingWidth;
				else if (heightAlongAxis == 'z') m.position.z = initialPos.z - dirZ * editingDepth;
			}
			mtx.unlock();
			std::cout << "models size after: " << modelsToRender.size() << std::endl;

			// std::cout << "model placed has sn: " << editingModel->sn << std::endl;
			editingModel = nullptr;
			isEdited = true;
			//std::cout << "added cube!" << cameraFront.x << cameraFront.y << cameraFront.z << std::endl;

		// right mouse click deletes the model currently in focus
		}else if (deletingModel == nullptr && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] && modelsInFocus.size() > 0) {
			mtx.lock();
			auto modelInFocus = *(modelsInFocus.begin());
			deletingModel = modelInFocus;
			mtx.unlock();

		}else if (deletingModel != nullptr && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK]==false) {
			std::cout << "deleted!" << std::endl;
			mtx.lock();
			deletingModel->removeFlag = true;
			unsigned long i = 0;
			unsigned long lastModelSn = 0;
			unsigned long lastCubeSn = 0;
			for (i; i < modelsToRender.size(); i++)
			{
				if (modelsToRender[i].removeFlag) break;
			}
			std::cout << "removed model with index = " << i << " and sn = " << modelsToRender[i].sn << std::endl;
			cubePointsCnt -= deletingModel->modelMesh.tris.size() * 3;
			for (i+=1; i < modelsToRender.size(); i++)
			{
				if (deletingModel->modelMesh.shape == Shape::CUBE && modelsToRender[i].modelMesh.shape == Shape::CUBE)
				{
					std::cout << "index = " << i << " sn = " << modelsToRender[i].sn << " - " << deletingModel->modelMesh.tris.size() * 3 << std::endl;
					modelsToRender[i].sn -= deletingModel->modelMesh.tris.size() * 3;
				}else if (deletingModel->modelMesh.shape != Shape::CUBE && modelsToRender[i].modelMesh.shape != Shape::CUBE)
				{
					modelsToRender[i].sn -= deletingModel->modelMesh.tris.size() * 3;
				}
			}
			modelsToRender.erase(std::remove_if(modelsToRender.begin(), modelsToRender.end(), [](model m) { return m.removeFlag == true; }), modelsToRender.end());
			
			mtx.unlock();
			deletingModel = nullptr;
			isEdited = true;
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
		updateVerticesFlag = isEdited;
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

	mtx.lock();
	std::vector<model> modelsToRenderCopy = modelsToRender;
	mtx.unlock();

	//populate vertex vectors with triangle vertex information for each model
	for (auto &model : modelsToRenderCopy)
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

	// std::cout << "  vertex data size: " << vertexData.size() << std::endl;
	// std::cout << "  index data size: " << indexData.size() << std::endl;
	// std::cout << "  cube vertex data size: " << cubeVertexData.size() << std::endl;
	// std::cout << "  cube index data size: " << cubeIndexData.size() << std::endl;

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
	if (updateVerticesFlag) updateVertices();

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

	// finalModelsToRender.clear();
	// mtx.lock();
	// for (model& model : modelsToRender)
	// {
	// 	if (model.isInDOF) finalModelsToRender.insert(&model);
	// }
	// mtx.unlock();

	mtx.lock();
	std::vector<model> modelsToRenderCopy = modelsToRender;
	mtx.unlock();

	for (model& model : modelsToRenderCopy)
	//for (auto itr = finalModelsToRender.begin(); itr != finalModelsToRender.end(); itr++)
	{
		if (!model.distance) continue;

		if (model.modelMesh.shape == Shape::CUBE)
		{
			//exclude out-of-range models
			if (!model.isInDOF) { break; }

			cubeMapShader->bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *gCubeIBO);
			glBindVertexArray(*gCubeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, (*cubemapIdsMap)[model.texture]);
			cubeMapShader->setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)((model.sn) * sizeof(GL_UNSIGNED_INT)));
		}
		else
		{
			//exclude out-of-range models
			if (!model.isInDOF) { break; }

			textureShader->bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *gIBO);
			glBindVertexArray(*gVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, (*textureIdsMap)[model.texture]);
			textureShader->setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(( model.sn ) * sizeof(GL_UNSIGNED_INT)));
		
		}
		glBindVertexArray(0);
	}
}

void Engine3D::captureInput()
{
	//listen for input
	std::memcpy(prevKeysPressed, keysPressed, SupportedKeys::ALL_KEYS * sizeof(bool));
	std::memcpy(keysPressed, eventController->getKeysPressed(), SupportedKeys::ALL_KEYS * sizeof(bool));
}

void Engine3D::setLevel(Level* level)
{
	modelsToRender = level->models;
	modelPointsCnt = level->modelPointsCnt;
	cubePointsCnt = level->cubePointsCnt;
}

std::string Engine3D::shapeToString(Shape s)
{
	switch (s) {
		case Shape::TRIANGLE:
			return "triangle";
		case Shape::RECTANGLE:
			return "rectangle";
		case Shape::CUBOID:
			return "cuboid";
		case Shape::CUBE:
			return "cube";
		default:
			return "";
	}
}
