#include "Engine3D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Engine3D::Engine3D(SDL_Window* gWindow,
				   int width, int height,
				   float near, float far,
				   float fov, float dof,
				   float collidingDistance,
				   float gravitationalPull,
				   float cameraSpeedFactor,
				   UserMode userMode, EventController* ec)
				   : gWindow(gWindow),
				   width(width), height(height),
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

	cubemapFaceIndexMap["right"] = 0;
	cubemapFaceIndexMap["left"] = 1;
	cubemapFaceIndexMap["top"] = 2;
	cubemapFaceIndexMap["bottom"] = 3;
	cubemapFaceIndexMap["back"] = 4;
	cubemapFaceIndexMap["front"] = 5;
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
	std::thread t = std::thread(&Engine3D::renderingThread, this);

	return t;
}

void Engine3D::renderingThread()
{
	//create context
	printf( "Creating OpenGL context...\n" );
	gContext = SDL_GL_CreateContext( gWindow );
	if( gContext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return;
	}
	else
	{
		//initialize GLEW
		printf( "Initializing GLEW...\n" );
		glewExperimental = GL_TRUE; 
		GLenum glewError = glewInit();
		if( glewError != GLEW_OK )
		{
			printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
			return;
		}

		if( !initGL() )
		{
			printf( "Unable to initialize OpenGL!\n" );
			return;
		}
	}
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

	rendererThread = startRendering();

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
	//for (auto &model : modelsToRender)
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;
		model& model = *ptrModel;

		mtx.lock();
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, model.position);
		model.modelMatrix = modelMatrix;

		model.inFocus = false;
		bool checkAgainForFocus = true;
		model.isInFOV = false;
		bool checkAgainForFOV = true;
		float minX = 1.0f, maxX = -1.0f, minY = 1.0f, maxY = -1.0f, minZ = 100000.0f, maxZ = -100000.0f;

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
				modelsInFocus.insert(ptrModel);
				mtx.unlock();
				checkAgainForFocus = false;
			}

			//calculate bounding box
			minX = std::min(minX, std::min(std::min(pt[0].x/pt[0].w, pt[1].x/pt[1].w), pt[2].x/pt[2].w));
			minY = std::min(minY, std::min(std::min(pt[0].y/pt[0].w, pt[1].y/pt[1].w), pt[2].y/pt[2].w));
			minZ = std::min(minZ, std::min(std::min(pt[0].z, pt[1].z), pt[2].z));
			maxX = std::max(maxX, std::max(std::max(pt[0].x/pt[0].w, pt[1].x/pt[1].w), pt[2].x/pt[2].w));
			maxY = std::max(maxY, std::max(std::max(pt[0].y/pt[0].w, pt[1].y/pt[1].w), pt[2].y/pt[2].w));
			maxZ = std::max(maxZ, std::max(std::max(pt[0].z, pt[1].z), pt[2].z));
		}

		//assign bounding box, to check for coverage later
		minX = (std::max(-1.0f, minX) + 1) / 2.0f;
		minY = (std::max(-1.0f, minY) + 1) / 2.0f;
		maxX = (std::min(1.0f, maxX) + 1) / 2.0f;
		maxY = (std::min(1.0f, maxY) + 1) / 2.0f;
		//std::cout << "X: " << minX << " - "<< maxX << ", Y: " << minY << " - " << maxY << ", Z: " << minZ << " - " << maxZ << std::endl;
		boundingbox bbox = { minX, maxX, minY, maxY, minZ, maxZ };
		model.bbox = bbox;

		//mark out-of-FOV models to avoid needless rendering
		model.isInFOV = ((model.bbox.minX > 0 && model.bbox.minX < 1) || (model.bbox.maxX > 0 && model.bbox.maxX < 1)) && ((model.bbox.minY > 0 && model.bbox.minY < 1) || (model.bbox.maxY > 0 && model.bbox.maxY < 1));

		//mark out-of-DOF models to avoid needless rendering
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

	
	//mark covered models to avoid needless rendering
	mtx.lock();
	markCoveredModels();
	mtx.unlock();

	mtx.lock();
	for (auto &ptrModel : ptrModelsToRender)
	{
		if (!ptrModel) continue;
		if (!ptrModel->isCovered && ptrModel->isInDOF && ptrModel->isInFOV)
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.insert(ptrModel);
			else finalModelsToRender.insert(ptrModel);
		}else
		{
			if (ptrModel->modelMesh.shape == shapetype::CUBE) finalCubeModelsToRender.erase(ptrModel);
			else finalModelsToRender.erase(ptrModel);
		}
	}
	//std::cout << "final models to render: " << finalCubeModelsToRender.size() << std::endl;
	mtx.unlock();

	edit(elapsedTime);

	// std::cout << "collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
	return true;
}

void Engine3D::markCoveredModels()
{
	for (auto &ptrCube1 : ptrModelsToRender)
	{
		if (!ptrCube1) continue;
		//if (!ptrCube1->isInDOF || !ptrCube1->isInFOV) continue;
		model& model1 = *ptrCube1;
		for (auto &ptrCube2 : ptrModelsToRender)
		{
			if (!ptrCube2) continue;
			//if (!ptrCube2->isInDOF || !ptrCube2->isInFOV) continue;
			model& model2 = *ptrCube2;
			model1.isCovered = model2.bbox.minX > 0 && model2.bbox.maxX < 1 &&
							   model2.bbox.minY > 0 && model2.bbox.maxY < 1 &&
							   model1.bbox.minX > model2.bbox.minX &&
							   model1.bbox.maxX < model2.bbox.maxX &&
							   model1.bbox.minY > model2.bbox.minY &&
							   model1.bbox.maxY < model2.bbox.maxY &&
							   model1.bbox.maxZ > model2.bbox.minZ;

			if (model1.isCovered) break;

		}
		//std::cout << "model " << model1.id << " is covered? " << model1.isCovered << std::endl;
	}
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
		//std::cout << "mouseDistX: " << mouseDistanceX << ", mouseDistY: " << mouseDistanceY << std::endl;
		//std::cout << "left: " << keysPressed[SupportedKeys::MOUSE_LEFT] << ", right: " << keysPressed[SupportedKeys::MOUSE_RIGHT] << ", up: " << keysPressed[SupportedKeys::MOUSE_UP] << ", down: " << keysPressed[SupportedKeys::MOUSE_DOWN] << std::endl;
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

		// pressing LCTRL + mouse wheel up/down cycles through edit options
		if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (--editOptionIndex > editOptions.size() - 1) editOptionIndex = editOptions.size() - 1;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		} else if (keysPressed[SupportedKeys::LEFT_CTRL] && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (++editOptionIndex > editOptions.size() - 1) editOptionIndex = 0;
			std::cout << "editing: " << editOptions[editOptionIndex] << std::endl;

		// increases/decreases collation height
		} else if (editOptions[editOptionIndex] == "collationHeight" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationHeight = std::max(--collationHeight, (unsigned int)1);
			std::cout << "collation height: " << collationHeight << std::endl;
		} else if (editOptions[editOptionIndex] == "collationHeight" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationHeight++;
			std::cout << "collation height: " << collationHeight << std::endl;

		// increases/decreases collation width
		} else if (editOptions[editOptionIndex] == "collationWidth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			collationWidth = std::max(--collationWidth, (unsigned int)1);
			std::cout << "collation width: " << collationWidth << std::endl;
		} else if (editOptions[editOptionIndex] == "collationWidth" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			collationWidth++;
			std::cout << "collation width: " << collationWidth << std::endl;

		// cycles through shapes
		} else if (editOptions[editOptionIndex] == "shape" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			if (++edShapeInt > shapetype::CUBE) edShapeInt = 0;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;
		} else if (editOptions[editOptionIndex] == "shape" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			if (--edShapeInt > shapetype::CUBE) edShapeInt = shapetype::CUBE;
			editingShape = (shapetype)edShapeInt;
			std::cout << "shape: " << shapeTypeToString(editingShape) << std::endl;

		// increases/decreases width
		} else if (editOptions[editOptionIndex] == "width" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingWidth = std::max(editingWidth - 0.1f, 0.1f);
			std::cout << "width: " << editingWidth << std::endl;
		} else if (editOptions[editOptionIndex] == "width" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingWidth += 0.1f;
			std::cout << "width: " << editingWidth << std::endl;

		// increases/decreases X rotation
		} else if (editOptions[editOptionIndex] == "rotationX" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false) {
			editingRotationX = std::max(editingRotationX - 0.1f, 0.0f);
			std::cout << "rotationX: " << editingRotationX << std::endl;
		} else if (editOptions[editOptionIndex] == "rotationX" && prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP] && keysPressed[SupportedKeys::MOUSE_WHEEL_UP] == false) {
			editingRotationX += 0.1f;
			std::cout << "rotationX: " << editingRotationX << std::endl;
		}

		if (editingModel == nullptr && keysPressed[SupportedKeys::MOUSE_LEFT_CLICK]) {
			cube cube(std::max(editingWidth, std::max(editingHeight, editingDepth)), editingRotationX, 0, 0);
			glm::vec3 position = cameraPos + (editingDepth + originalCollidingDistance) * cameraFront;
			cubeModel mdl(0, cubePointsCnt, "box", position, cube, true);
			cubePointsCnt += mdl.modelMesh.tris.size() * 3;
			std::cout << "about to place model with sn = " << mdl.sn << std::endl;
			mtx.lock();
			if (modelsInFocus.size() > 0) { modelInFocusTmp = **(modelsInFocus.begin()); }
			//ptrModelsToRender.push_back(std::make_shared<model>(mdl));
			ptrModelsToRender.push_back(std::make_shared<cubeModel>(mdl));
			editingModel = ptrModelsToRender.back();
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
				//model* m2 = &modelsToRender.back();
				//cubePointsCnt -= m2->sn;
				cubePointsCnt -= ptrModelsToRender.back()->sn;
				//modelsToRender.pop_back();
				ptrModelsToRender.pop_back();
				m.sn = cubePointsCnt;
				cubePointsCnt += m.modelMesh.tris.size() * 3;
				//modelsToRender.push_back(m);
				ptrModelsToRender.push_back(std::make_shared<model>(m));
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
			//std::cout << "models size before: " << modelsToRender.size() << std::endl;
			std::cout << "models size before: " << ptrModelsToRender.size() << std::endl;
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
					//modelsToRender.push_back(m);
					ptrModelsToRender.push_back(std::make_shared<cubeModel>(m));
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
			//std::cout << "models size after: " << modelsToRender.size() << std::endl;
			std::cout << "models size after: " << ptrModelsToRender.size() << std::endl;

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
			//TODO AFTER PTR CUBES TO RENDER IS DONE
			// for (i; i < modelsToRender.size(); i++)
			// {
			// 	if (modelsToRender[i].removeFlag) break;
			// }
			// std::cout << "removed model with index = " << i << " and sn = " << modelsToRender[i].sn << std::endl;
			// cubePointsCnt -= deletingModel->modelMesh.tris.size() * 3;
			// for (i+=1; i < modelsToRender.size(); i++)
			// {
			// 	if (deletingModel->modelMesh.shape == shapetype::CUBE && modelsToRender[i].modelMesh.shape == shapetype::CUBE)
			// 	{
			// 		std::cout << "index = " << i << " sn = " << modelsToRender[i].sn << " - " << deletingModel->modelMesh.tris.size() * 3 << std::endl;
			// 		modelsToRender[i].sn -= deletingModel->modelMesh.tris.size() * 3;
			// 	}else if (deletingModel->modelMesh.shape != shapetype::CUBE && modelsToRender[i].modelMesh.shape != shapetype::CUBE)
			// 	{
			// 		modelsToRender[i].sn -= deletingModel->modelMesh.tris.size() * 3;
			// 	}
			// }
			// modelsToRender.erase(std::remove_if(modelsToRender.begin(), modelsToRender.end(), [](model m) { return m.removeFlag == true; }), modelsToRender.end());
			
			mtx.unlock();
			deletingModel = nullptr;
			isEdited = true;
		}

		
		// if (modelsInFocus.size())
		// {
		// 	if (prevModelInFocus != *(modelsInFocus.begin()))
		// 	{
		// 		auto modelInFocus = *(modelsInFocus.begin());
		// 		for (triangle& tri : modelInFocus->modelMesh.tris) 
		// 		{
		// 			tri.R = 0; tri.G = 192; tri.B = 0;
		// 		}
		// 		if (prevModelInFocus != nullptr)
		// 		{
		// 			for (triangle& tri : prevModelInFocus->modelMesh.tris)
		// 			{
		// 				tri.R = 255; tri.G = 255; tri.B = 255;
		// 			}
		// 		}
		// 		//updateVerticesFlag = true;
		// 	}
		// }
		updateVerticesFlag = isEdited;
	}

}

bool Engine3D::onUserDestroy()
{
	std::cout << "Destroying Engine3D..." << std::endl;

	printf("Stopping rendering thread...\n");

	rendererThread.join();

	printf("Unbinding and deleting shader programs...\n");

	//unbind program - deactivate shader
	cubeMapShader.unbind();
	textureShader.unbind();

	//deallocate programs
	cubeMapShader.freeProgram();
	textureShader.freeProgram();

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
	for (auto &ptrModel : ptrModelsToRender)
	//for (auto &model : modelsToRenderCopy)
	{
		if (!ptrModel) continue;
		model& model = *ptrModel;

		vdp = model.modelMesh.shape == shapetype::CUBE ? &cubeVertexData : &vertexData;

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
				if (model.modelMesh.shape != shapetype::CUBE)
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
	glBindVertexArray(gVAO);
	glBindBuffer( GL_ARRAY_BUFFER, gVBO );
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
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GL_UNSIGNED_INT), indexData.data(), GL_STATIC_DRAW );

	//update cubeVBO
	glBindVertexArray(gCubeVAO);
	glBindBuffer( GL_ARRAY_BUFFER, gCubeVBO );
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
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gCubeIBO );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, cubeIndexData.size() * sizeof(GL_UNSIGNED_INT), cubeIndexData.data(), GL_STATIC_DRAW );

	updateVerticesFlag = false;

	std::cout << "Updated vertices" << std::endl;
}

void Engine3D::render()
{
	if (updateVerticesFlag) updateVertices();
	//if (finalCubeModelsToRender.size() == 0) return;

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cubeMapShader.bind();
	cubeMapShader.setMat4("projection", getProjectionMatrix());
	cubeMapShader.setMat4("view", getViewMatrix());
	//lighting
	cubeMapShader.setVec3("lightPos", getLightPos());
	cubeMapShader.setVec3("viewPos", getCameraPos());
	cubeMapShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	cubeMapShader.unbind();

	textureShader.bind();
	textureShader.setMat4("projection", getProjectionMatrix());
	textureShader.setMat4("view", getViewMatrix());
	textureShader.unbind();
	// //lighting
	// textureShader->setVec3("lightPos", getLightPos());
	// textureShader->setVec3("viewPos", getCameraPos());
	// textureShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	// mtx.lock();
	// std::vector<model> modelsToRenderCopy = modelsToRender;
	// mtx.unlock();

	cubeMapShader.bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gCubeIBO);
	glBindVertexArray(gCubeVAO);
	glActiveTexture(GL_TEXTURE0);
	int cnt = 0;
	mtx.lock();
	//std::cout << "about to render " << finalCubeModelsToRender.size() << " out of " << ptrModelsToRender.size() << " models" << std::endl;
	
	//for(const auto& model : ptrModelsToRender)
	for (auto itr = finalCubeModelsToRender.begin(); itr != finalCubeModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }
		
		model& model = *(*itr);
		model.render(&cubeMapShader, cubemapIdsMap[model.texture]);
		cnt++;
	}
	glBindVertexArray(0);
	cubeMapShader.unbind();

	textureShader.bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
	glBindVertexArray(gVAO);
	glActiveTexture(GL_TEXTURE0);
	for (auto itr = finalModelsToRender.begin(); itr != finalModelsToRender.end(); itr++)
	{
		if (!(*itr)) { std::cout << "nullptr!" << std::endl; continue; }

		model& model = *(*itr);
		model.render(&textureShader, textureIdsMap[model.texture]);
		cnt++;
	}
	glBindVertexArray(0);
	textureShader.unbind();
	mtx.unlock();

	//update screen
	SDL_GL_SwapWindow( gWindow );
}

void Engine3D::captureInput()
{
	//listen for input
	std::memcpy(prevKeysPressed, keysPressed, SupportedKeys::ALL_KEYS * sizeof(bool));
	std::memcpy(keysPressed, eventController->getKeysPressed(), SupportedKeys::ALL_KEYS * sizeof(bool));
}

void Engine3D::setLevel(Level* level)
{
	for (model& m : level->models)
	{
		if (m.modelMesh.shape == shapetype::CUBE)
		{
			ptrModelsToRender.push_back(std::make_shared<cubeModel>(m));
		}
		else
		{
			ptrModelsToRender.push_back(std::make_shared<model>(m));
		}
	}
	modelPointsCnt = level->modelPointsCnt;
	cubePointsCnt = level->cubePointsCnt;
}

std::string Engine3D::shapeTypeToString(shapetype s)
{
	switch (s) {
		case shapetype::RECTANGLE:
			return "rectangle";
		case shapetype::CUBOID:
			return "cuboid";
		case shapetype::CUBE:
			return "cube";
		default:
			return "";
	}
}

bool Engine3D::initGL()
{
	bool success = true;

	printf( "Loading shader programs...\n" );
	if (!cubeMapShader.loadProgram("shaders/cubemap.glvs", "shaders/cubemap.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	else if(!textureShader.loadProgram("shaders/texture.glvs", "shaders/texture.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS); 

		gCubeMapProgramID = cubeMapShader.getProgramID();
		gTextureProgramID = textureShader.getProgramID();
		
		//create VAOs
		glGenVertexArrays(1, &gVAO);
		glGenVertexArrays(1, &gCubeVAO);

		//create VBOs
		glGenBuffers( 1, &gVBO );
		glGenBuffers( 1, &gCubeVBO );

		//create IBO
		glGenBuffers( 1, &gIBO );
		glGenBuffers( 1, &gCubeIBO );

		//update buffers with the new vertices
		updateVertices();

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures
		loadTextures(textureIdsMap);

		//generates and binds cubemap
		loadCubemaps(cubemapIdsMap);
		
	}
	return success;
}

void Engine3D::loadTextures(std::map<std::string, GLuint>& textureIdsMap)
{
	std::string texturesPath = cfg.ASSETS_PATH + std::string("\\textures");
	std::string filename;
	for (const auto & entry : std::filesystem::directory_iterator(texturesPath))
	{
		if (entry.is_regular_file())
		{
			if (entry.path().filename().has_extension()) {
				filename = entry.path().filename().replace_extension("").string();
			} else {
				filename = entry.path().filename().string();
			}

			textureIdsMap[filename] = -1;
			//declare texture
			glGenTextures(1, &textureIdsMap[filename]);
			//bind texture
			glBindTexture(GL_TEXTURE_2D, textureIdsMap[filename]);

			//set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//load image
			int width, height, nrChannels;
			unsigned char *data = stbi_load(entry.path().string().c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
				//generate texture
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			} else {
				std::cout << "Failed to load texture " << filename << std::endl;
			}
			//free image memory
			stbi_image_free(data);
		}
	}
	//activate shader
	textureShader.bind();
	//set the uniforms
	for (const auto& kv : textureIdsMap) {
		glUniform1i(glGetUniformLocation(textureShader.getProgramID(), std::string("texture" + std::to_string(kv.second)).c_str()), 0);
	}
}

void Engine3D::loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap)
{
    std::string cubemapsPath = cfg.ASSETS_PATH + std::string("\\cubemaps");
	std::string filename;
	std::string name;
    for (const auto & entry : std::filesystem::directory_iterator(cubemapsPath))
	{
		if (entry.is_directory())
		{
			name = entry.path().filename().string();

			cubemapIdsMap[name] = -1;
			glGenTextures(1, &cubemapIdsMap[name]);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdsMap[name]);

			for (const auto & face : std::filesystem::directory_iterator(entry))
			{
				if (face.is_regular_file())
				{
					if (face.path().filename().has_extension()) {
						filename = face.path().filename().replace_extension("").string();
					} else {
						filename = face.path().filename().string();
					}

					int width, height, nrChannels;
					int i = 0;
					if (cubemapFaceIndexMap[filename] >= 0)
					{
						unsigned char *data = stbi_load(face.path().string().c_str(), &width, &height, &nrChannels, 0);
						if (data)
						{
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapFaceIndexMap[filename], 
											0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
							);
							stbi_image_free(data);
						}
						else
						{
							std::cout << "Cubemap texture failed to load at path: " << face.path() << std::endl;
							stbi_image_free(data);
						}
					}
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					cubeMapShader.bind();
					glUniform1i(glGetUniformLocation(cubeMapShader.getProgramID(), std::string("cubemap" + std::to_string(cubemapIdsMap[name])).c_str()), 0);
				}
			}
		}
	}
}
