#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ArtificeShaderProgram.h"
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
#include <map>

class Engine3D
{
	public:

		Engine3D(int width = 320, int height = 240,
				float near = 0.1f, float far = 1000.0f,
				float fov = 90.0f, float dof = 20.0f,
				float collidingDistance = 0.2f, float gravitationalPull = 0.1f,
				EventController* eventController = nullptr);

		std::thread startEngine();

		bool onUserCreate();

		bool onUserUpdate(float elapsedTime);

		virtual bool onUserDestroy();

		glm::mat4 getProjectionMatrix() const;

		glm::vec3 getCameraPos() const;

		glm::vec3 getCameraFront() const;

		glm::vec3 getCameraUp() const;

		glm::vec3 getLightPos() const;

		glm::mat4 getViewMatrix() const;

		void updateVertices(GLuint* gVBO, GLuint* gIBO, GLuint* gVAO, GLuint* gCubeVBO, GLuint* gCubeIBO, GLuint* gCubeVAO);

		void render(ArtificeShaderProgram* textureShader,
					std::map<std::string, GLuint>* textureIdsMap,
					ArtificeShaderProgram* cubeMapShader,
					std::map<std::string, GLuint>* cubemapIdsMap,
					GLuint* gVAO,
					GLuint* gCubeVAO);

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
		float dof;

		//for collide-and-slide
		float collidingDistance;
		bool canSlide = false;
		bool collides = false;
		bool collidesFront = false;
		bool collidesBack = false;
		bool collidesRight = false;
		bool collidesLeft = false;

		float gravitationalPull;
		bool hasLanded = false;

		glm::vec3 desiredMotion;

		glm::mat4 projectionMatrix;

		float yaw = -90.0f;

		float pitch = 0;

		//camera
		glm::vec3 cameraPos;

		glm::vec3 cameraFront;

		glm::vec3 cameraUp;

		glm::vec3 cameraRight;

		glm::vec3 lightPos;

		glm::mat4 viewMatrix;

		EventController* eventController;

		void engineThread();

		void move(float elapsedTime);

};