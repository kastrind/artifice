#pragma once

//Using SDL, SDL OpenGL, GLEW, stb_image, GLM, standard IO, and strings
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <string>
#include <map>
#include <filesystem>

#include "ArtificeShaderProgram.h"
#include "Configuration.h"
#include "EventController.h"
#include "Constructs3D.h"
#include "Engine3D.h"


class Initiator
{
	public:

		Initiator(CFG& cfg) : cfg(cfg)
		{
			windowRect.x = cfg.SCREEN_WIDTH/4;
			windowRect.y = cfg.SCREEN_HEIGHT/4;
			windowRect.w = cfg.SCREEN_WIDTH/2;
			windowRect.h = cfg.SCREEN_HEIGHT/2;
			cubemapFaceIndexMap["right"] = 0;
			cubemapFaceIndexMap["left"] = 1;
			cubemapFaceIndexMap["top"] = 2;
			cubemapFaceIndexMap["bottom"] = 3;
			cubemapFaceIndexMap["back"] = 4;
			cubemapFaceIndexMap["front"] = 5;
		}

		bool initiated = false;

		//configuration
		CFG cfg;

		//the window we'll be rendering to
		SDL_Window* gWindow = NULL;

		//OpenGL context
		SDL_GLContext gContext;

		//window mouse barrier
		SDL_Rect windowRect;

		//the game engine
		Engine3D* artificeEngine = NULL;

		//the engine thread
		std::thread engineThread;

		//shader programs
		ArtificeShaderProgram textureShader;
		ArtificeShaderProgram cubeMapShader;

		//graphics program
		GLuint gCubeMapProgramID = 0;
		GLuint gTextureProgramID = 0;
		GLuint gVBO = 0;
		GLuint gIBO = 0;
		GLuint gVAO = 0;
		GLuint gCubeVBO = 0;
		GLuint gCubeIBO = 0;
		GLuint gCubeVAO = 0;

		//declare textures
		std::vector<GLuint> textureIds;

		GLuint cubemapTexture;

		std::vector<std::string> texturePaths;
		std::map<std::string, GLuint> textureIdsMap;
		std::vector<std::string> cubemapPaths;
		std::map<std::string, GLuint> cubemapIdsMap;

		//input event controller
		EventController eventController;

		//overload function call operator
    	bool operator()() {
      		return init();
    	}

		//starts up SDL, creates window, and initializes OpenGL
		bool init();

		//initializes rendering program and clear color
		bool initGL();

		//generates and binds textures
		void loadTextures(std::map<std::string, GLuint>& textureIdsMap);
		
		//generates and binds a cubemap
		void loadCubemaps(std::map<std::string, GLuint>& cubemapIdsMap);

		//frees media and shuts down SDL
		void close();


	private:

		std::map<std::string, unsigned int> cubemapFaceIndexMap;

};