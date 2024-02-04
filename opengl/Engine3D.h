#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Configuration.h"
#include "Constructs3D.h"
#include "EventController.h"

#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
class Engine3D
{
	public:

		Engine3D(int width=320, int height=240, float near = 0.1f, float far = 1000.0f, float fov = 90.0f, EventController* eventController = nullptr);

		std::thread startEngine();

		bool onUserCreate();

		bool onUserUpdate(float elapsedTime);

		virtual bool onUserDestroy();

		glm::mat4 getProjectionMatrix() const;

		glm::vec3 getCameraPos() const;

		glm::vec3 getCameraFront() const;

		glm::vec3 getCameraUp() const;

		glm::mat4 getViewMatrix() const;

		std::atomic<bool> isActive;

		std::atomic<bool> isTouched;

		float elapsedTime;

		std::mutex mtx;

		std::vector<model> modelsToRaster;

	private:

		int width;
		int height;
		float near;
		float far;
		float fov;
		float dp = 0.0f; //TODO: remove/refactor
		float absDP = 0.0f; //TODO: remove/refactor
		float canSlide = false; //TODO: remove/refactor
		float dist = 100.0f; //TODO: remove/refactor
		bool collides = false;
		glm::vec3 desiredMotion;

		glm::mat4 projectionMatrix;

		float yaw = -90.0f;

		float pitch = 0;

		mat4x4 matCameraRotY90CW;

		mat4x4 matCameraRotY90CCW;

		//camera
		glm::vec3 cameraPos;

		glm::vec3 cameraFront;

		glm::vec3 cameraUp;

		glm::mat4 viewMatrix;

		EventController* eventController;

		void engineThread();

		void move(float elapsedTime);
};