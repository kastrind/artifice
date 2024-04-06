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
#include "Level.h"
#include "EventController.h"

#include <vector>
#include <set>
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
#include <memory>

class Engine3D
{
	public:

		Engine3D(int width = 320, int height = 240,
				float near = 0.1f, float far = 1000.0f,
				float fov = 90.0f, float dof = 20.0f,
				float collidingDistance = 0.2f,
				float gravitationalPull = 0.1f,
				float cameraSpeedFactor = 1.5f,
				UserMode userMode = UserMode::PLAYER,
				EventController* eventController = nullptr);

		std::thread startEngine();

		std::thread startRendering();

		bool onUserCreate();

		bool onUserUpdate(float elapsedTime);

		virtual bool onUserDestroy();

		glm::mat4 getProjectionMatrix() const;

		glm::vec3 getCameraPos() const;

		glm::vec3 getCameraFront() const;

		glm::vec3 getCameraUp() const;

		glm::vec3 getLightPos() const;

		glm::mat4 getViewMatrix() const;

		void updateVertices();

		void render();

		void renderingLoop();

		void setLevel(Level* level);

		//to render
		ArtificeShaderProgram* textureShader = nullptr;
		std::map<std::string, GLuint>* textureIdsMap = nullptr;
		ArtificeShaderProgram* cubeMapShader = nullptr;
		std::map<std::string, GLuint>* cubemapIdsMap = nullptr;

		//to update vertices
		GLuint* gVBO = nullptr;
		GLuint* gIBO = nullptr;
		GLuint* gVAO = nullptr;
		GLuint* gCubeVBO = nullptr;
		GLuint* gCubeIBO = nullptr;
		GLuint* gCubeVAO = nullptr;

		std::atomic<bool> isActive;

		std::atomic<bool> updateVerticesFlag;

		float elapsedTime;

		std::mutex mtx;

		std::vector<model> modelsToRender;
		std::vector<std::shared_ptr<model>> ptrCubesToRender;

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

		UserMode userMode;

		EventController* eventController;

		bool hasLanded = false;

		glm::vec3 desiredMotion;

		glm::mat4 projectionMatrix;

		float yaw = -90.0f;

		float pitch = 0;

		struct ModelDistanceComparator {
			bool operator()(const model* a, const model* b) const { return a->distance < b->distance; };
		};
		std::set<model*, ModelDistanceComparator> modelsInFocus;
		model* prevModelInFocus = nullptr;

		std::set<std::shared_ptr<model>> finalModelsToRender;



		//editor user mode specific
		float originalCollidingDistance;
		unsigned int collationHeight = 1;
		unsigned int collationWidth = 1;
		float editingWidth = 0;
		float editingHeight = 0;
		float editingDepth = 0;
		Shape editingShape = Shape::CUBE; unsigned short edShapeInt = 1;
		std::shared_ptr<model> editingModel = nullptr;
		model* deletingModel = nullptr;
		model modelInFocusTmp;
		unsigned long modelPointsCnt = 0;
		unsigned long cubePointsCnt = 0;

		bool keysPressed[SupportedKeys::ALL_KEYS];
		bool prevKeysPressed[SupportedKeys::ALL_KEYS];

		//camera
		glm::vec3 cameraPos;

		glm::vec3 cameraFront;

		glm::vec3 cameraUp;

		glm::vec3 cameraRight;

		float cameraSpeedFactor;

		glm::vec3 lightPos;

		glm::mat4 viewMatrix;

		void engineThread();

		void inputListenerThread();

		void move(float elapsedTime);

		void edit(float elapsedTime);

		void captureInput();

		std::string shapeToString(Shape s);

		void markCoveredModels();

};