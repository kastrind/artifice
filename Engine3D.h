#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ArtificeShaderProgram.h"
#include "Configuration.h"
#include "Constructs3D.h"
#include "Level.h"
#include "EventController.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <filesystem>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <memory>

class Engine3D
{
	public:

		Engine3D(SDL_Window* gWindow,
				int width = 320, int height = 240,
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

		void setLevel(Level* level);

		std::atomic<bool> isActive;

	private:

		//the window
		SDL_Window* gWindow;

		//OpenGL context
		SDL_GLContext gContext;

		//shader programs
		ArtificeShaderProgram textureShader;
		ArtificeShaderProgram cubeMapShader;

		//to render
		std::vector<std::string> texturePaths;
		std::map<std::string, GLuint> textureIdsMap;
		std::vector<std::string> cubemapPaths;
		std::map<std::string, GLuint> cubemapIdsMap;

		//to update vertices
		GLuint gCubeMapProgramID = 0;
		GLuint gTextureProgramID = 0;
		GLuint gVBO = 0;
		GLuint gIBO = 0;
		GLuint gVAO = 0;
		GLuint gCubeVBO = 0;
		GLuint gCubeIBO = 0;
		GLuint gCubeVAO = 0;

		std::atomic<bool> updateVerticesFlag;

		float elapsedTime;

		std::mutex mtx;

		std::vector<model> modelsToRender;

		std::vector<std::shared_ptr<model>> ptrModelsToRender;

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

		std::set<std::shared_ptr<model>> finalCubeModelsToRender;
		std::set<std::shared_ptr<model>> finalModelsToRender;

		//editor user mode specific
		float originalCollidingDistance;
		unsigned int collationHeight = 1;
		unsigned int collationWidth = 1;
		float editingWidth = 0;
		float editingHeight = 0;
		float editingDepth = 0;
		shapetype editingShape = shapetype::CUBE; unsigned short edShapeInt = 1;
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

		std::map<std::string, unsigned int> cubemapFaceIndexMap;
				
		std::thread rendererThread;

		bool initGL();

		void loadTextures(std::map<std::string, GLuint>& textureIdsMap);

		void loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap);

		void updateVertices();

		void engineThread();

		void renderingThread();

		void render();

		void move(float elapsedTime);

		void edit(float elapsedTime);

		void captureInput();

		std::string shapeTypeToString(shapetype s);

		void markCoveredModels();

};