#pragma once

/*
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
*/

#define _USE_MATH_DEFINES
#include <cmath>

#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp> // required for glm::angle

#include "ArtificeShaderProgram.h"
#include "Configuration.h"
#include "Constructs3D.h"
#include "Light.h"
#include "Level.h"
#include "EventController.h"
#include "Preset.h"
#include "Utility.h"

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

typedef enum camerapositionmode {
	ATTACHED_TO_PERSON,
	FIXED
} camerapositionmode;

class Camera
{
	public:

		camerapositionmode positionMode = camerapositionmode::ATTACHED_TO_PERSON;
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 offset   = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 front    = glm::vec3(0.0f, 0.0f, -1.0f);

		Camera(camerapositionmode positionMode, glm::vec3 position, glm::vec3 offset, glm::vec3 front)
			: positionMode(positionMode), position(position), offset(offset), front(front) {}

		Camera() {}

		virtual ~Camera() {}
};

class Engine3D
{
	public:

		std::atomic<bool> isActive;

		Engine3D(
				SDL_Window* gWindow, Camera camera,
				int width = 320, int height = 240,
				float near = 0.1f, float far = 1000.0f,
				float fov = 90.0f, float dof = 20.0f,
				float collidingDistanceH = 0.2f,
				float collidingDistanceV = 0.1f,
				float gravitationalPull = 0.1f,
				float jumpSpeedFactor = 5.0f,
				float personSpeedFactor = 1.5f,
				UserMode userMode = UserMode::PLAYER,
				EventController* eventController = nullptr
				);

		std::thread startEngine();

		bool update(float elapsedTime);

		bool stopEngine(std::thread& engineThreadToJoin);

		glm::mat4 getProjectionMatrix() const;

		void setCameraPos(glm::vec3 pos);

		glm::vec3 getCameraPos() const;

		void setCameraOffset(glm::vec3 pos);

		glm::vec3 getCameraOffset() const;

		void setCameraFront(glm::vec3 pos);

		glm::vec3 getCameraFront() const;

		void setCameraUp(glm::vec3 pos);

		glm::vec3 getCameraUp() const;

		void setCameraRight(glm::vec3 pos);

		glm::vec3 getCameraRight() const;

		void setPersonPos(glm::vec3 pos);

		glm::vec3 getPersonPos() const;

		void setPersonFront(glm::vec3 pos);

		glm::vec3 getPersonFront() const;

		void setPersonUp(glm::vec3 pos);

		glm::vec3 getPersonUp() const;

		void setPersonRight(glm::vec3 pos);

		glm::vec3 getPersonRight() const;

		glm::vec3 getLightPos() const;

		glm::mat4 getViewMatrix() const;

		glm::mat4 getViewMatrixNoTranslation() const;

		void setLevel(Level* level);

	private:

		//the window
		SDL_Window* gWindow;

		//OpenGL context
		SDL_GLContext gContext;

		//shader programs to implement a 3-step shader pipeline for deferred rendering: geometry pass, lighting pass, post-processing pass
		ArtificeShaderProgram geometryShader;
		ArtificeShaderProgram geometryCubemapShader;
		ArtificeShaderProgram geometrySkyboxShader;
		ArtificeShaderProgram lightingShader;
		ArtificeShaderProgram postProcShader;

		//textures, lightmaps, normalmaps, displacementmaps
		std::vector<std::string> texturePaths;
		std::map<std::string, GLuint> textureIdsMap;
		std::map<std::string, GLuint> lightmapIdsMap;
		std::map<std::string, GLuint> normalmapIdsMap;
		std::map<std::string, GLuint> displacementmapIdsMap;
		std::map<std::string, bool> textureTransparencyMap;
		std::vector<std::string> textureNames;

		std::vector<std::string> cubemapPaths;
		std::map<std::string, GLuint> cubemapIdsMap;
		std::map<std::string, GLuint> cubeLightmapIdsMap;
		std::map<std::string, GLuint> cubeNormalmapIdsMap;
		std::map<std::string, GLuint> cubeDisplacementmapIdsMap;
		std::vector<std::string> cubemapNames;

		//shader IDs
		GLuint gGeometryProgramID = 0;
		GLuint gGeometryCubemapProgramID = 0;
		GLuint gGeometrySkyboxProgramID = 0;
		GLuint gLightingProgramID = 0;
		GLuint gPostProcProgramID = 0;

		//buffer object IDs
		GLuint gVBO = 0;
		GLuint gIBO = 0;
		GLuint gVAO = 0;
		GLuint gCubeVBO = 0;
		GLuint gCubeIBO = 0;
		GLuint gCubeVAO = 0;

		GLuint gBOMS = 0; //G-Buffer for MSAA
		GLuint gPositionMS = 0; //position color buffer texture for MSAA
		GLuint gNormalMS = 0; //normal color buffer texture for MSAA
		GLuint gAlbedoMS = 0; //color buffer texture for MSAA
		GLuint gLightmapMS = 0; //normal map color buffer texture for MSAA
		GLuint gViewDirMS = 0; //view dir color buffer texture for MSAA
		GLuint depthRenderBOMS = 0; //depth buffer (renderbuffer) for MSAA

		GLuint gBO = 0; //G-Buffer for deferred rendering
		GLuint gPosition = 0; //position color buffer texture
		GLuint gNormal = 0; //normal color buffer texture
		GLuint gAlbedo = 0; //color buffer texture
		GLuint gLightmap = 0; //normal map color buffer texture
		GLuint gViewDir = 0; //view dir color buffer texture
		GLuint depthRenderBO = 0; //depth buffer (renderbuffer)

		GLuint lightingBO = 0; //lighting buffer for post-processing
		GLuint screenTexture = 0; //lighting buffer texture, input to the post-processing step
		GLuint scrQuadVAO = 0;
		GLuint scrQuadVBO = 0;

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
		float fovH;
		float fovHalf;
		float fovHHalf;
		float dof;

		//for collide-and-slide
		float collidingDistanceH;
		float collidingDistanceV;
		bool canSlide = false;
		bool collides = false;
		bool collidesFront = false;
		bool collidesBack = false;
		bool collidesRight = false;
		bool collidesLeft = false;

		float gravitationalPull;
		float jumpSpeedFactor;
		float jumpSpeed = 0;
		glm::vec3 personFrontOnJump;

		UserMode userMode;

		EventController* eventController;

		bool hasLanded = false;

		bool shouldClimb = false;

		glm::vec3 desiredMotion;

		glm::mat4 projectionMatrix;

		float yaw = -90.0f;

		float pitch = 0;

		struct ModelDistanceComparator {
			bool operator()(const std::shared_ptr<model>& a, const std::shared_ptr<model>& b) const { return (a != b) ? a->position != b->position && a->distance < b->distance : a.get() < b.get(); };
		};
		struct ModelDescendingDistanceComparator {
			bool operator()(const std::shared_ptr<model>& a, const std::shared_ptr<model>& b) const { return (a != b) ?  a->position != b->position && a->distance > b->distance : a.get() > b.get(); };
		};
		std::set<std::shared_ptr<model>, ModelDistanceComparator> modelsInFocus;

		std::shared_ptr<cubeModel> finalSkyBoxToRender = nullptr;
		std::set<std::shared_ptr<model>> finalCubeModelsToRender;
		std::set<std::shared_ptr<model>> finalModelsToRender;
		std::set<std::shared_ptr<model>, ModelDescendingDistanceComparator> finalTransparentModelsToRender;

		glm::vec3 lightPos;

		Light light;
		Light selectedLight;

		PointLight pointLight;

		std::vector<PointLight> pointLights;

		SpotLight spotLight;

		std::vector<SpotLight> spotLights;

		SpotLight flashLight;

		bool assignedFlashLight = false;

		bool isFlashLightOn = false;

		//editor user mode specific
		std::vector<std::string> editOptions = {"shape", "width", "height", "depth", "rotationX", "rotationY", "rotationZ", "texture", "isSolid", "collationHeight", "collationWidth"};
		unsigned short editOptionIndex = 0;
		std::vector<std::string> lightingEditOptions = {"light type", "preset light"};
		unsigned short lightingEditOptionIndex = 0;
		std::vector<std::string> lightingTypeOptions = {};
		unsigned short lightingTypeOptionIndex = 0;
		unsigned short presetDirectionalLightIndex = 0;
		unsigned short presetPointLightIndex = 0;
		unsigned short presetSpotLightIndex = 0;
		float originalCollidingDistanceH = 0;
		float originalCollidingDistanceV = 0;
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
		bool isLightingEditingModeEnabled = false;
		Preset preset;

		bool isModelEditingModeEnabled = false;
		unsigned long compoundModelId = 0;

		bool keysPressed[SupportedKeys::ALL_KEYS];
		bool prevKeysPressed[SupportedKeys::ALL_KEYS];

		std::shared_ptr<Level> level = nullptr;

		//person
		glm::vec3 personPos;

		glm::vec3 personFront;

		glm::vec3 personUp;

		glm::vec3 personRight;

		float personSpeedFactor;

		//camera
		Camera camera;

		glm::vec3 cameraPos;

		glm::vec3 cameraOffset;

		glm::vec3 cameraFront;

		glm::vec3 cameraUp;

		glm::vec3 cameraRight;

		glm::mat4 viewMatrix;

		std::map<std::string, unsigned int> cubemapFaceIndexMap;
				
		std::thread renderingThread;

		void engineLoop();

		std::thread startRendering();

		void renderingLoop();

		bool initGL();

		void loadTextures(std::map<std::string, GLuint>& textureIdsMap, std::map<std::string, GLuint>& lightmapIdsMap, std::map<std::string, GLuint>& normalmapIdsMap, std::map<std::string, GLuint>& displacementmapIdsMap);

		void loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap, std::map<std::string, GLuint>& cubeLightmapIdsMap, std::map<std::string, GLuint>& cubeNormalmapIdsMap, std::map<std::string, GLuint>& cubeDisplacementmapIdsMap);

		//bool initUI();

		void render();

		void renderScreenQuad();

		void updateVertices();

		//void renderUI();

		bool isInFOV(model& m);

		void captureInput();

		void markCoveredModels();

		void move(float elapsedTime);

		//editor user mode specific

		void addModel(float editingWidth, float editingHeight, float editingDepth, float editingRotationX, float editingRotationY, float editingRotationZ, unsigned int editingCubemapNameIndex, unsigned int editingTextureNameIndex, bool editingIsSolid, shapetype type, glm::vec3 position);

		void addModel(model& mdl);

		void addLightHandleModel(unsigned long id, glm::vec3 position, glm::mat4 rotationMatrix = glm::mat4(1.0f));

		void removeModel(std::shared_ptr<model> m);

		void edit(float elapsedTime);

		int64_t getTimeSinceEpoch();

		std::string shapeTypeToString(shapetype s);

};