//Using SDL, SDL OpenGL, GLEW, stb_image, GLM, standard IO, and strings
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <string>
#include <map>

#include "ArtificeShaderProgram.h"
#include "Configuration.h"
#include "EventController.h"
#include "Constructs3D.h"
#include "Engine3D.h"

CFG cfg;

//screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//starts up SDL, creates window, and initializes OpenGL
bool init();

//initializes rendering program and clear color
bool initGL();

//generates and binds textures
void loadTextures(std::vector<std::string> texturePaths, std::map<std::string, GLuint>& textureIdsMap);

//generates and binds a cubemap
void loadCubemap(std::vector<std::string> cubemapPaths, std::map<std::string, GLuint>& cubemapIdsMap, std::string name); 

//per frame update
void updateVertices();

//renders to the screen
void render();

//frees media and shuts down SDL
void close();

//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//graphics program
GLuint gCubeMapProgramID = 0;
GLuint gTextureProgramID = 0;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;
GLuint gCubeVBO = 0;
GLuint gCubeIBO = 0;
GLuint gCubeVAO = 0;

//shader programs
ArtificeShaderProgram textureShader;
ArtificeShaderProgram cubeMapShader;

//declare textures
std::vector<GLuint> textureIds;

GLuint cubemapTexture;

std::vector<std::string> texturePaths;
std::map<std::string, GLuint> textureIdsMap;
std::vector<std::string> cubemapPaths;
std::map<std::string, GLuint> cubemapIdsMap;

//window mouse barrier
SDL_Rect windowRect{cfg.SCREEN_WIDTH/4, cfg.SCREEN_HEIGHT/4, cfg.SCREEN_WIDTH/2, cfg.SCREEN_HEIGHT/2};

//the game engine
Engine3D* artificeEngine;

//the engine thread
std::thread engineThread;

//input event controller
EventController eventController;

bool init()
{
	//initialization flag
	bool success = true;

	//initialize SDL
	printf( "Initializing SDL...\n" );
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//use OpenGL 3.1 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

		//create window
		printf( "Creating window...\n" );
		gWindow = SDL_CreateWindow( "Artifice Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
		//confine mouse cursor to the window and hide it
		SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
		SDL_SetWindowMouseRect(gWindow, &windowRect);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//create context
			printf( "Creating OpenGL context...\n" );
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//initialize GLEW
				printf( "Initializing GLEW...\n" );
				glewExperimental = GL_TRUE; 
				GLenum glewError = glewInit();
				if( glewError != GLEW_OK )
				{
					printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
				}

				//use Vsync
				printf( "Setting VSync...\n" );
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//instantiate the game engine
				artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);

				//initialize OpenGL
				printf( "Initializing OpenGL...\n" );
				if( !initGL() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}
			}
		}
	}

	return success;
}

bool initGL()
{
	bool success = true;

	printf( "Loading shader programs...\n" );
	if (!cubeMapShader.loadProgram("shaders/cubemap.glvs", "shaders/cubemap.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	else if(!textureShader.loadProgram("shaders/texture.glvs", "shaders/texture.glfs"))
	{
		printf( "Unable to load cubemap shader!\n" );
		success = false;
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		// glDepthFunc(GL_LESS); 
		// glEnable(GL_CULL_FACE);
		// glCullFace(GL_BACK);

		gCubeMapProgramID = cubeMapShader.getProgramID();
		gTextureProgramID = textureShader.getProgramID();

		//create a rectangle
		rectangle rect0{0.2, 0.4};
		model mdl0; mdl0.texture = "brickwall.bmp";
		mdl0.position = glm::vec3( -0.7f,  0.5f,  0.2f);
		rect0.toTriangles(mdl0.modelMesh.tris);
		mdl0.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0);

		//create a rectangle
		rectangle rect0b{0.2, 0.2};
		model mdl0b; mdl0b.texture = "walnut.bmp";
		mdl0b.position = glm::vec3( 0.2f,  0.7f,  0.5f);
		rect0b.toTriangles(mdl0b.modelMesh.tris);
		mdl0b.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0b);

		rectangle rect0c{0.2, 0.2};
		model mdl0c; mdl0c.texture = "brickwallPainted.bmp";
		mdl0c.position = glm::vec3( 0.5f,  0.1f,  0.1f);
		rect0c.toTriangles(mdl0c.modelMesh.tris);
		mdl0c.modelMesh.shape = Shape::RECTANGLE;
		artificeEngine->modelsToRaster.push_back(mdl0c);

		//create a cube
		cube cube0{0.2f};
		std::vector<triangle> cube0Triangles;
		cube0.toTriangles(cube0Triangles);

		//create models
		model mdl; mdl.texture = "cubemap";
		mdl.position = glm::vec3( 0.0f,  0.0f,  0.0f);
		mdl.modelMesh.tris = cube0Triangles;
		mdl.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl);

		model mdl2; mdl2.texture = "cubemap";
		mdl2.position = glm::vec3( 0.0f,  0.0f,  0.2f);
		mdl2.modelMesh.tris = cube0Triangles;
		mdl2.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl2);

		model mdl3; mdl3.texture = "cubemap";
		mdl3.position = glm::vec3( -0.2f,  0.2f,  0.0f);
		mdl3.modelMesh.tris = cube0Triangles;
		mdl3.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl3);

		model mdl4; mdl4.texture = "cubemap";
		mdl4.position = glm::vec3( -0.2f,  0.2f, 0.2f);
		mdl4.modelMesh.tris = cube0Triangles;
		mdl4.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl4);

		model mdl5; mdl5.texture = "cubemap";
		mdl5.position = glm::vec3( 0.0f,  0.0f, 0.4f);
		mdl5.modelMesh.tris = cube0Triangles;
		mdl5.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl5);

		model mdl6; mdl6.texture = "cubemap";
		mdl6.position = glm::vec3( 0.2f,  0.2f, 0.0f);
		mdl6.modelMesh.tris = cube0Triangles;
		mdl6.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl6);

		model mdl7; mdl7.texture = "cubemap";
		mdl7.position = glm::vec3( 0.2f,  0.2f, 0.2f);
		mdl7.modelMesh.tris = cube0Triangles;
		mdl7.modelMesh.shape = Shape::CUBE;
		artificeEngine->modelsToRaster.push_back(mdl7);
		
		//create VAOs
		glGenVertexArrays(1, &gVAO);
		glGenVertexArrays(1, &gCubeVAO);

		//create VBOs
		glGenBuffers( 1, &gVBO );
		glGenBuffers( 1, &gCubeVBO );

		//create IBO
		glGenBuffers( 1, &gIBO );
		glGenBuffers( 1, &gCubeIBO );

		//update buffers with the new vertices
		updateVertices();

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures
		loadTextures(texturePaths, textureIdsMap);

		//generates and binds cubemap
		loadCubemap(cubemapPaths, cubemapIdsMap, "cubemap");
	}
	return success;
}

void loadTextures(std::vector<std::string> texturePaths, std::map<std::string, GLuint>& textureIdsMap)
{
	for (std::string texturePath : texturePaths)
	{
		textureIdsMap[texturePath] = -1;
		//textureIds.push_back(0);
		//declare texture
		glGenTextures(1, &textureIdsMap[texturePath]);
		//bind texture
		glBindTexture(GL_TEXTURE_2D, textureIdsMap[texturePath]);

		//set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//load image
		int width, height, nrChannels;
		unsigned char *data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			//generate texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture " << texturePath << std::endl;
		}
		//free image memory
		stbi_image_free(data);
	}
	//activate shader
	textureShader.bind();
	//set the uniforms
	int cnt=0;
	for (const auto& kv : textureIdsMap) {
		std::cout << kv.first << ": " << kv.second << std::endl;
		glUniform1i(glGetUniformLocation(textureShader.getProgramID(), std::string("texture" + std::to_string(kv.second)).c_str()), 0);
	}
	// for (GLuint& textureId : textureIds)
	// {
	// 	glUniform1i(glGetUniformLocation(textureShader.getProgramID(), std::string("texture" + std::to_string(textureId)).c_str()), cnt++);
	// }
}

void loadCubemap(std::vector<std::string> cubemapPaths, std::map<std::string, GLuint>& cubemapIdsMap, std::string name)
{
	cubemapIdsMap[name] = -1;
	glGenTextures(1, &cubemapIdsMap[name]);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdsMap[name]);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < cubemapPaths.size(); i++)
	{
		unsigned char *data = stbi_load(cubemapPaths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
							0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << cubemapPaths[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	cubeMapShader.bind();
	glUniform1i(glGetUniformLocation(cubeMapShader.getProgramID(), std::string("cubemap" + std::to_string(cubemapIdsMap[name])).c_str()), 0);
}

void updateVertices()
{
		std::vector<GLfloat> vertexData;
		std::vector<GLuint> indexData;
		GLuint indexCounter = 0;

		std::vector<GLfloat> cubeVertexData;
		std::vector<GLuint> cubeIndexData;
		GLuint cubeIndexCounter = 0;

		std::vector<GLfloat>* vdp = &vertexData;

		//populate vertex vectors with triangle vertex information for each model
		for (auto &model : artificeEngine->modelsToRaster)
		{

			vdp = model.modelMesh.shape == Shape::CUBE ? &cubeVertexData : &vertexData;

			for (auto &tri : model.modelMesh.tris)
			{
				glm::vec3 line1 = tri.p[1] - tri.p[0];
				glm::vec3 line2 = tri.p[2] - tri.p[0];
				glm::vec3 normal = glm::normalize(glm::cross(line1, line2));

				for (int i = 0; i < 3; i++)
				{
					vdp->push_back(tri.p[i].x);
					vdp->push_back(tri.p[i].y);
					vdp->push_back(tri.p[i].z);
					vdp->push_back(normal.x);
					vdp->push_back(normal.y);
					vdp->push_back(normal.z);
					vdp->push_back((float)tri.R/255.0f);
					vdp->push_back((float)tri.G/255.0f);
					vdp->push_back((float)tri.B/255.0f);
					vdp->push_back(tri.t[i].x);
					vdp->push_back(tri.t[i].y);
					if (model.modelMesh.shape != Shape::CUBE)
					{
						indexData.push_back(indexCounter++);
					}
					else
					{
						cubeIndexData.push_back(cubeIndexCounter++);
					}
				}
			}
		}

		std::cout << "vertex data size: " << vertexData.size() << std::endl;
		std::cout << "index data size: " << indexData.size() << std::endl;
		std::cout << "cube vertex data size: " << cubeVertexData.size() << std::endl;
		std::cout << "cube index data size: " << cubeIndexData.size() << std::endl;

		// glBindVertexArray(gVAO);
		// glBindVertexArray(gCubeVAO);

		// glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		// glBindBuffer( GL_ARRAY_BUFFER, gCubeVBO );

		// glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		// glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gCubeIBO );

		//update VBO
		glBindVertexArray(gVAO);
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW );

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//color attribute
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		//texture coord attribute
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);
		// //texture id attribute
		// glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(8 * sizeof(float)));
		// glEnableVertexAttribArray(3);

		//update IBO
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW );

		//update cubeVBO
		glBindVertexArray(gCubeVAO);
		glBindBuffer( GL_ARRAY_BUFFER, gCubeVBO );
		glBufferData( GL_ARRAY_BUFFER, cubeVertexData.size() * sizeof(GLfloat), cubeVertexData.data(), GL_STATIC_DRAW );

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//normal attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//color attribute
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		//texture coord attribute
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		//update cubeIBO
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gCubeIBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, cubeIndexData.size() * sizeof(GLuint), cubeIndexData.data(), GL_STATIC_DRAW );
}

void render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, textureIds.front());

	cubeMapShader.bind();
	cubeMapShader.setMat4("projection", artificeEngine->getProjectionMatrix());
	cubeMapShader.setMat4("view", artificeEngine->getViewMatrix());
	//lighting
	cubeMapShader.setVec3("lightPos", artificeEngine->getLightPos());
	cubeMapShader.setVec3("viewPos", artificeEngine->getCameraPos());
	cubeMapShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);

	textureShader.bind();
	textureShader.setMat4("projection", artificeEngine->getProjectionMatrix());
	textureShader.setMat4("view", artificeEngine->getViewMatrix());
	//lighting
	textureShader.setVec3("lightPos", artificeEngine->getLightPos());
	textureShader.setVec3("viewPos", artificeEngine->getCameraPos());
	textureShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	
	unsigned int modelCnt = 0;
	unsigned int cubeCnt = 0;
	unsigned int prevModelTrisSize = 0;
	unsigned int prevCubeTrisSize = 0;
	for (auto &model : artificeEngine->modelsToRaster)
	{

		if (model.modelMesh.shape == Shape::CUBE)
		{
			//ignore out-of-range models
			if (!model.isInDOF) { cubeCnt++; prevCubeTrisSize = model.modelMesh.tris.size(); continue; }

			cubeMapShader.bind();
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIBO);
			glBindVertexArray(gCubeVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapIdsMap[model.texture]);
			cubeMapShader.setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(((cubeCnt++) * (prevCubeTrisSize * 3) ) * sizeof(float)));
			prevCubeTrisSize = model.modelMesh.tris.size();
		}
		else
		{
			//ignore out-of-range models
			if (!model.isInDOF) { modelCnt++; prevModelTrisSize = model.modelMesh.tris.size(); continue; }

			textureShader.bind();
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gCubeIBO);
			glBindVertexArray(gVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureIdsMap[model.texture]);
			textureShader.setMat4("model", model.modelMatrix);
			glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(((modelCnt++) * (prevModelTrisSize * 3) ) * sizeof(float)));
			prevModelTrisSize = model.modelMesh.tris.size();
		}
		glBindVertexArray(0);
	}
}

void close()
{
	//unbind program - deactivate shader
	cubeMapShader.unbind();

	//deallocate programs
	glDeleteProgram(gCubeMapProgramID);
	glDeleteProgram(gTextureProgramID);

	//destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	engineThread.join();

	//quit SDL subsystems
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	//TODO: relocate
	texturePaths.push_back("brickwall.bmp");
	texturePaths.push_back("brickwallPainted.bmp");
	texturePaths.push_back("walnut.bmp");

	cubemapPaths.push_back("brickwallPainted.bmp"); //right
	cubemapPaths.push_back("brickwall.bmp"); //left
	cubemapPaths.push_back("brickwallPainted.bmp"); //top
	cubemapPaths.push_back("brickwall.bmp"); //bottom
	cubemapPaths.push_back("brickwallPainted.bmp"); //back
	cubemapPaths.push_back("brickwall.bmp"); //front

	//start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//start the 3D engine
		engineThread = artificeEngine->startEngine();

		//main loop flag
		bool quit = false;

		//event handler
		SDL_Event e;
		
		//enable text input
		SDL_StartTextInput();

		//while application is running
		while( !quit )
		{

			eventController.clearMouseMotionState();

			//handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//user requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
					artificeEngine->isActive = false;
				}else if (e.key.keysym.sym == SDLK_ESCAPE && SDL_GetWindowMouseGrab(gWindow) == SDL_TRUE) {
					//free mouse cursor from the window and reveal it
					SDL_SetWindowMouseGrab(gWindow, SDL_FALSE);
					SDL_SetWindowMouseRect(gWindow, NULL);
					SDL_SetRelativeMouseMode(SDL_FALSE);
				}else if (e.type == SDL_MOUSEBUTTONDOWN && SDL_GetWindowMouseGrab(gWindow) == SDL_FALSE) {
					//confine mouse cursor to the window and hide it
					SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
					SDL_SetWindowMouseRect(gWindow, &windowRect);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				//user presses or releases a key
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
				{
					eventController.processEvent(&e);
				}
				//just a temporary proof-of-concept to modify world on user input
				if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_0) {
					for (auto &model : artificeEngine->modelsToRaster) {
						std::cout << "removing triangle! ! !" << std::endl;
						if (model.modelMesh.tris.size()) model.modelMesh.tris.pop_back();
						artificeEngine->isTouched = true;
					}
				}
			}

			//just a temporary proof-of-concept to update vertices when world is modified
			if (artificeEngine->isTouched)
			{
				updateVertices();
				artificeEngine->isTouched = false;
			}

			//render
			render();

			//update screen
			SDL_GL_SwapWindow( gWindow );
		}

		//disable text input
		SDL_StopTextInput();
	}

	//free resources and close SDL
	close();

	return 0;
}
