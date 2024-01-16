#pragma once

#include "Configuration.h"
#include "Constructs3D.h"
#include "EventController.h"
#define _USE_MATH_DEFINES
#include <cmath>
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

		mat4x4 getProjMatrix();

		void clearDepthBuffer();

		std::atomic<bool> isActive;

		std::atomic<bool> blockRaster;

		float elapsedTime;

		std::mutex mtx;

		std::vector<triangle> trianglesToRaster;

		std::vector<model> modelsToRaster;

	private:

		int width;
		int height;
		float near;
		float far;
		float fov;
		float aspectRatio;
		float fovRad;

		mat4x4 matProj;

		float* depthBuffer = nullptr;

		float theta = 0;

		float yaw = 0;

		float pitch = 0;

		mat4x4 matCameraRotY90CW;

		mat4x4 matCameraRotY90CCW;

		vec3d lookDir;

		vec3d up;

		vec3d camera;

		vec3d target;

		vec3d forward;

		vec3d right;

		vec3d left;

		vec3d light;

		EventController* eventController;

		void engineThread();

		void fillProjMatrix();

		void move();

		std::list<triangle>* clip(triangle tri);

};