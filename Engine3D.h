#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"

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
#include "Light.h"
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

		std::atomic<bool> isActive;

		Engine3D(
				SDL_Window* gWindow,
				int width = 320, int height = 240,
				float near = 0.1f, float far = 1000.0f,
				float fov = 90.0f, float dof = 20.0f,
				float collidingDistance = 0.2f,
				float gravitationalPull = 0.1f,
				float jumpSpeedFactor = 5.0f,
				float cameraSpeedFactor = 1.5f,
				UserMode userMode = UserMode::PLAYER,
				EventController* eventController = nullptr
				);

		std::thread startEngine();

		bool onUserCreate();

		bool onUserUpdate(float elapsedTime);

		bool onUserDestroy();

		glm::mat4 getProjectionMatrix() const;

		glm::vec3 getCameraPos() const;

		glm::vec3 getCameraFront() const;

		glm::vec3 getCameraUp() const;

		glm::vec3 getLightPos() const;

		glm::mat4 getViewMatrix() const;

		glm::mat4 getViewMatrixNoTranslation() const;

		void setLevel(Level* level);

	private:

		//the window
		SDL_Window* gWindow;

		//OpenGL context
		SDL_GLContext gContext;

		//shader programs
		ArtificeShaderProgram textureShader;
		ArtificeShaderProgram cubeMapShader;
		ArtificeShaderProgram skyBoxShader;

		//to render
		std::vector<std::string> texturePaths;
		std::map<std::string, GLuint> textureIdsMap;
		std::map<std::string, bool> textureTransparencyMap;
		std::vector<std::string> textureNames;

		std::vector<std::string> cubemapPaths;
		std::map<std::string, GLuint> cubemapIdsMap;
		std::vector<std::string> cubemapNames;

		//to update vertices
		GLuint gCubeMapProgramID = 0;
		GLuint gSkyBoxProgramID = 0;
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
		float jumpSpeedFactor;
		float jumpSpeed = 0;
		glm::vec3 cameraFrontOnJump;

		UserMode userMode;

		EventController* eventController;

		bool hasLanded = false;

		glm::vec3 desiredMotion;

		glm::mat4 projectionMatrix;

		float yaw = -90.0f;

		float pitch = 0;

		struct ModelDistanceComparator {
			bool operator()(std::shared_ptr<model> a, std::shared_ptr<model> b) const { return a->distance < b->distance; };
		};
		struct ModelDescendingDistanceComparator {
			bool operator()(std::shared_ptr<model> a, std::shared_ptr<model> b) const { return a->distance > b->distance; };
		};
		std::set<std::shared_ptr<model>, ModelDistanceComparator> modelsInFocus;
		std::shared_ptr<model> prevModelInFocus = nullptr;

		std::shared_ptr<cubeModel> finalSkyBoxToRender = nullptr;
		std::set<std::shared_ptr<model>> finalCubeModelsToRender;
		std::set<std::shared_ptr<model>> finalModelsToRender;
		std::set<std::shared_ptr<model>, ModelDescendingDistanceComparator> finalTransparentModelsToRender;

		glm::vec3 lightPos;

		Light light;

		//editor user mode specific
		std::vector<std::string> editOptions = {"shape", "width", "height", "depth", "rotationX", "rotationY", "rotationZ", "texture", "isSolid", "collationHeight", "collationWidth"};
		unsigned short editOptionIndex = 0;
		float originalCollidingDistance;
		unsigned int collationHeight = 1;
		unsigned int collationWidth = 1;
		float editingWidth = 0.1f;
		float editingHeight = 0.1f;
		float editingDepth = 0.1f;
		float editingRotationX = 0.0f;
		float editingRotationY = 0.0f;
		float editingRotationZ = 0.0f;
		unsigned int editingTextureNameIndex = 0;
		unsigned int editingCubemapNameIndex = 0;
		shapetype editingShape = shapetype::CUBE;
		unsigned short edShapeInt = 1;
		bool editingIsSolid = true;
		std::shared_ptr<model> editingModel = nullptr;
		std::shared_ptr<model> deletingModel = nullptr;
		std::shared_ptr<model> copyingModel = nullptr;
		model modelInFocusTmp;
		unsigned long modelPointsCnt = 0;
		unsigned long cubePointsCnt = 0;

		bool keysPressed[SupportedKeys::ALL_KEYS];
		bool prevKeysPressed[SupportedKeys::ALL_KEYS];

		std::shared_ptr<Level> level = nullptr;

		//camera
		glm::vec3 cameraPos;

		glm::vec3 cameraFront;

		glm::vec3 cameraUp;

		glm::vec3 cameraRight;

		float cameraSpeedFactor;

		glm::mat4 viewMatrix;

		std::map<std::string, unsigned int> cubemapFaceIndexMap;
				
		std::thread rendererThread;

		void engineThread();

		std::thread startRendering();

		void renderingThread();

		bool initGL();

		void loadTextures(std::map<std::string, GLuint>& textureIdsMap);

		void loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap);

		bool initUI();

		void render();

		void updateVertices();

		void renderUI();

		void captureInput();

		void markCoveredModels();

		void move(float elapsedTime);


		//editor user mode specific

		void addModel(float editingWidth, float editingHeight, float editingDepth, float editingRotationX, float editingRotationY, float editingRotationZ, unsigned int editingCubemapNameIndex, unsigned int editingTextureNameIndex, bool editingIsSolid, shapetype type, glm::vec3 position);

		void addModel(model& mdl);

		void removeModel(std::shared_ptr<model> m);

		void edit(float elapsedTime);

		int64_t getTimeSinceEpoch();

		std::string shapeTypeToString(shapetype s);

};