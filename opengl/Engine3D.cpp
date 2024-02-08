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

	viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	matCameraRotY90CW = mat4x4::getRotMatrixY(-cfg.M_PI_HALF);
	matCameraRotY90CCW = mat4x4::getRotMatrixY(cfg.M_PI_HALF);
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

	//std::cout << "collides: " << collides << std::endl;
	//std::cout << "can slide: " << canSlide << std::endl;
	//std::cout << "desiredMotion: " << desiredMotion.x << ", " << desiredMotion.y << ", " << desiredMotion.z << ", " << std::endl;

	glm::vec3 prevCameraPos;
	float modelDistance = 100.0f;
	float prevCollidingDistance = 100.0f;
	glm::vec4 collidingTriPts[3];

	move(elapsedTime);

	collides = false;
	canSlide = false;

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

		modelDistance = 100.0f;

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
			if (avgDist < modelDistance) {
				modelDistance =  avgDist;
				collidingTriPts[0] = pt[0];
				collidingTriPts[1] = pt[1];
				collidingTriPts[2] = pt[2];
			}

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
			vec3d center{ 0, 0, 0 };
			model.inFocus = tri.contains(center);
		}

		//detect if colliding with the model
		if (modelDistance <= 0.2f) {
			prevCollidingDistance = collidingDistance;
			collidingDistance = modelDistance;
			//std::cout << "modelDistance: " << modelDistance << std::endl;
			collides = true;

			//get the triangle normal
			glm::vec4 line1 = collidingTriPts[1] - collidingTriPts[0];
			glm::vec4 line2 = collidingTriPts[2] - collidingTriPts[0];
			glm::vec3 normal = glm::normalize(glm::cross(glm::vec3(line1.x, line1.y, line1.z), glm::vec3(line2.x, line2.y, line2.z)));

			//based on dp and normal, determine if able to slide and the desired motion
			float dp = glm::dot(cameraFront, normal);
			float absDP = std::abs(dp);
			canSlide = absDP < 0.8f && absDP > 0.2f;
			glm::vec3 undesiredMotion = normal * dp;
			desiredMotion = cameraFront - undesiredMotion;
			desiredMotion = glm::normalize(desiredMotion);
		}

	}

	std::cout << "prevCollidingDistance: " << prevCollidingDistance << std::endl;
	std::cout << "collidingDistance: " << collidingDistance << std::endl;
	if (collides && !canSlide && prevCollidingDistance > collidingDistance)
	{
		cameraPos = prevCameraPos;
		prevCollidingDistance = collidingDistance;
	}else if (!collides || canSlide)
	{
		prevCameraPos = cameraPos;
	}

	return true;
}

void Engine3D::move(float elapsedTime)
{
	float cameraSpeed = static_cast<float>(1.5 * elapsedTime);

	if (eventController != nullptr)
	{
		bool* keysPressed = eventController->getKeysPressed();
		int mouseDistanceX = eventController->getMouseDistanceX();
		int mouseDistanceY = eventController->getMouseDistanceY();
		float multiplierX = (float)mouseDistanceX * 5;
		float multiplierY = (float)mouseDistanceY * 5;

		if (keysPressed[SupportedKeys::W] && collides && canSlide) {
			cameraPos += cameraSpeed * desiredMotion;
		} else if (keysPressed[SupportedKeys::W]) {
			cameraPos += cameraSpeed * cameraFront;
		} else if (keysPressed[SupportedKeys::S]) {
			cameraPos -= cameraSpeed * cameraFront;
		}

		if (keysPressed[SupportedKeys::A] && collides && canSlide) {
			cameraPos -= cameraSpeed * desiredMotion;
		}else if (keysPressed[SupportedKeys::A]) {
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		} else if (keysPressed[SupportedKeys::D] && collides && canSlide) {
			cameraPos -= cameraSpeed * desiredMotion;
		} else if (keysPressed[SupportedKeys::D]) {
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		}

		if (keysPressed[SupportedKeys::MOUSE_LEFT]) {
			yaw -= (multiplierX * elapsedTime) > 0.0f ? multiplierX * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::LEFT_ARROW]) {
			yaw -= cameraSpeed * 5;
		} else if (keysPressed[SupportedKeys::MOUSE_RIGHT]) {
			yaw += (multiplierX * elapsedTime) > 0.0f ? multiplierX * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::RIGHT_ARROW]) {
			yaw += cameraSpeed * 5;
		}

		if (keysPressed[SupportedKeys::MOUSE_UP]) {
			pitch += (multiplierY * elapsedTime) > 0.0f ? multiplierY * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::UP_ARROW]) {
			pitch += cameraSpeed * 5;
		} else if (keysPressed[SupportedKeys::MOUSE_DOWN]) {
			pitch -= (multiplierY * elapsedTime) > 0.0f ? multiplierY * elapsedTime : 0;
		} else if (keysPressed[SupportedKeys::DOWN_ARROW]) {
			pitch -= cameraSpeed * 5;
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
	return viewMatrix;
}

