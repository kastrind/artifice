#include "Engine3D.h"

Engine3D::Engine3D(int width, int height, float near, float far, float fov, EventController* ec)
				: width(width), height(height), near(near), far(far), fov(fov), eventController(ec)
{
	isActive = false;

	isTouched = false;

	projectionMatrix = glm::perspective(glm::radians((float)fov), (float)width / (float)height, near, far);

	//camera
	cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

std::thread Engine3D::startEngine()
{
	//start thread
	std::thread t = std::thread(&Engine3D::engineThread, this);

	return t;
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
	vec3d center{ 0, 0, 0 };
	float modelDistance = cfg.DOF;
	float maxModelDist = 0.0f;
	glm::vec4 collidingTriPts[3];

	move(elapsedTime);

	collides = false;
	collidesFront = false;
	collidesBack = false;
	collidesRight = false;
	collidesLeft = false;

	canSlide = false;
	hasLanded = false;

	float prevdpFront = 1.0f;

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
		mtx.unlock();

		modelDistance = cfg.DOF;

		for (auto tri : model.modelMesh.tris)
		{
			glm::vec4 pt[3];
			pt[0] = { tri.p[0].x, tri.p[0].y, tri.p[0].z, tri.p[0].w };
			pt[1] = { tri.p[1].x, tri.p[1].y, tri.p[1].z, tri.p[1].w };
			pt[2] = { tri.p[2].x, tri.p[2].y, tri.p[2].z, tri.p[2].w };

			//model transformation
			pt[0] = modelMatrix * pt[0];
			pt[1] = modelMatrix * pt[1];
			pt[2] = modelMatrix * pt[2];

			//finds distance from model
			glm::vec4 avgP = ( (pt[0]) + (pt[1]) + (pt[2]) ) / 3.0f;
			float avgDist = glm::distance(cameraPos, glm::vec3(avgP.x, avgP.y, avgP.z));
			float dist1 = glm::distance(cameraPos, glm::vec3(pt[0].x, pt[0].y, pt[0].z));
			float dist2 = glm::distance(cameraPos, glm::vec3(pt[1].x, pt[1].y, pt[1].z));
			float dist3 = glm::distance(cameraPos, glm::vec3(pt[2].x, pt[2].y, pt[2].z));
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
			model.inFocus = tri.contains(center);
		}

		//detect if colliding with the model
		if (model.isSolid && modelDistance <= cfg.COLLIDING_DISTANCE * 1.5f) {
			//std::cout << "modelDistance: " << modelDistance << std::endl;

			//get the triangle normal
			glm::vec4 line1 = collidingTriPts[1] - collidingTriPts[0];
			glm::vec4 line2 = collidingTriPts[2] - collidingTriPts[0];
			glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(line1.x, line1.y, line1.z), glm::vec3(line2.x, line2.y, line2.z)));
			//std::cout << normal.x << ", " << normal.y << ", " << normal.z << std::endl;

			//based on dp and normal, determine if able to slide and the desired motion
			float dpFront = glm::dot(cameraFront, normal);
			float absDP = std::abs(dpFront);
			float dpRight = glm::dot(cameraRight, normal);
			float dpLeft = glm::dot(-cameraRight, normal);
			float dpBack = glm::dot(-cameraFront, normal);

			//normal is up means collision with floor
			if (collidingTriPts[0].y <= cameraPos.y && normal.y > normal.z && normal.y > normal.x) {
				hasLanded = true;
			//else collision with wall
			}else if (modelDistance < cfg.COLLIDING_DISTANCE && maxModelDist < cfg.COLLIDING_DISTANCE * 1.5f) {
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
			prevdpFront = dpFront;
		}

		if (modelDistance < cfg.COLLIDING_DISTANCE * 0.5f)
		{
			cameraPos = prevCameraPos;
		}
	}
	std::cout << "collides? " << collides << ", canSlide? " << canSlide << ", hasLanded? " << hasLanded << std::endl;
	return true;
}

void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(1.5 * elapsedTime);

	cameraPos += !hasLanded ? cfg.GRAVITATIONAL_PULL * cameraSpeed * glm::vec3(0, -1, 0) : glm::vec3(0, 0, 0);

	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = (float)mouseDistanceX * 5;
		float multiplierY = (float)mouseDistanceY * 5;

		if (keysPressed[SupportedKeys::W] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::W] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::W] && !hasLanded) {
			cameraPos += cameraSpeed * cameraFront;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && collides && canSlide) {
			desiredMotion.y = 0;
			cameraPos -= 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::S] && hasLanded && !collides) {
			cameraFront.y = 0;
			cameraPos -= cameraSpeed * cameraFront;
		}

		if (keysPressed[SupportedKeys::A] && hasLanded && collides && !collidesLeft) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos += 0.5f * cameraSpeed * desiredMotion;
			else cameraPos -= 0.5f * cameraSpeed * desiredMotion;
		
		}else if (keysPressed[SupportedKeys::A] && hasLanded && !collides) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		
		} else if (keysPressed[SupportedKeys::D] && hasLanded && collides && !collidesRight) {
			desiredMotion.y = 0;
			bool desiresRight = glm::dot(cameraRight, desiredMotion) < 0;
			if (desiresRight) cameraPos -= 0.5f * cameraSpeed * desiredMotion;
			else cameraPos += 0.5f * cameraSpeed * desiredMotion;

		} else if (keysPressed[SupportedKeys::D] && hasLanded && !collides) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		if (keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw -= multiplierX * elapsedTime;
		} else if (keysPressed[SupportedKeys::LEFT_ARROW]) {
			yaw -= cameraSpeed * elapsedTime * 5;
		} else if (keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw += multiplierX * elapsedTime;
		} else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
			yaw += cameraSpeed * elapsedTime * 5;
		}

		if (keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += multiplierY * elapsedTime;
		} else if (keysPressed[SupportedKeys::UP_ARROW]) {
			pitch += cameraSpeed * elapsedTime * 5;
		} else if (keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= multiplierY * elapsedTime;
		} else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
			pitch -= cameraSpeed * elapsedTime * 5;
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

glm::mat4 Engine3D::getViewMatrix() const
{
	//glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(viewMatrix)); 
	//return viewWithoutTranslation;
	return viewMatrix;
}

