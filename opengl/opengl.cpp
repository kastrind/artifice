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

#include "LArtificeShaderProgram.h"
#include "Configuration.h"
#include "EventController.h"
#include "Constructs3D.h"
#include "Engine3D.h"

CFG cfg;

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes rendering program and clear color
bool initGL();

//Per frame update
void update();

//Renders to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;

LArtificeShaderProgram artificeShaderProgram;

//declare texture
unsigned int texture1;

//window mouse barrier
SDL_Rect windowRect{cfg.SCREEN_WIDTH/4, cfg.SCREEN_HEIGHT/4, cfg.SCREEN_WIDTH/2, cfg.SCREEN_HEIGHT/2};

//the graphics engine
Engine3D* artificeEngine;

//input event controller
EventController eventController;

bool isActive=true;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Use OpenGL 3.1 core
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
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
			//Create context
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize GLEW
				glewExperimental = GL_TRUE; 
				GLenum glewError = glewInit();
				if( glewError != GLEW_OK )
				{
					printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
				}

				//Use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//instantiate the 3D engine
				artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);

				//Initialize OpenGL
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
	//Success flag
	bool success = true;

	if (!artificeShaderProgram.loadProgram())
	{
		printf( "Unable to load basic shader!\n" );
		success = false;
	}
	else
	{
		glEnable(GL_DEPTH_TEST);

		gProgramID = artificeShaderProgram.getProgramID();

		model mdl;
		mdl.position = glm::vec3( 0.0f,  0.0f,  0.0f);
		//create a rectangle
		//rectangle rect{0, 0, 0, 1,    0.2, 0.2,    0.0, 0.0, 0.0};
		//create a cuboid
		cuboid box{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box.toTriangles(mdl.modelMesh.tris);

		artificeEngine->modelsToRaster.push_back(mdl);

		std::vector<GLfloat> vertexData;
		std::vector<GLuint> indexData;
		GLuint indexCounter = 0;

		for (auto &model : artificeEngine->modelsToRaster)
		{
			for (auto &tri : mdl.modelMesh.tris)
			{
				for (int i = 0; i < 3; i++)
				{
					vertexData.push_back(tri.p[i].x);
					vertexData.push_back(tri.p[i].y);
					vertexData.push_back(tri.p[i].z);
					vertexData.push_back((float)tri.R/255);
					vertexData.push_back((float)tri.G/255);
					vertexData.push_back((float)tri.B/255);
					vertexData.push_back(tri.t[i].u);
					vertexData.push_back(tri.t[i].v);
					indexData.push_back(indexCounter++);
				}
			}
		}

		glGenVertexArrays(1, &gVAO);
		glBindVertexArray(gVAO);

		std::cout << "vertex data size: " << vertexData.size() << std::endl;
		std::cout << "index data size: " << indexData.size() << std::endl;

		//Create VBO
		glGenBuffers( 1, &gVBO );
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		//glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
		glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW );

		//Create IBO
		glGenBuffers( 1, &gIBO );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), &indexData, GL_STATIC_DRAW );

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		//Initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//TODO: relocate the below
		//declare texture
		glGenTextures(1, &texture1); 
		//bind texture
		glBindTexture(GL_TEXTURE_2D, texture1);
		//set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//load image
		int width, height, nrChannels;
		unsigned char *data = stbi_load("brickwall.bmp", &width, &height, &nrChannels, 0);
		if (data)
		{
			std::cout << width << ", " << height << ", " << nrChannels << std::endl;
			//generate texture
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		//free image memory
		stbi_image_free(data);
		//activate shader
		artificeShaderProgram.bind();
		glUniform1i(glGetUniformLocation(artificeShaderProgram.getProgramID(), "texture1"), 0);
	}
	return success;
}

void update()
{
	//No per frame update needed
}

void render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

	//bind program - activate shader
	artificeShaderProgram.bind();

	//artificeEngine->mtx.lock();
	glm::mat4 projectionMatrix = glm::perspective(glm::radians((float)cfg.FOV), (float)cfg.SCREEN_WIDTH / (float)cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR);
	artificeShaderProgram.setMat4("projection", projectionMatrix);
	//artificeShaderProgram.setMat4("projection", artificeEngine->getProjectionMatrix());

	//camera/view transformation
	glm::mat4 viewMatrix = glm::lookAt(artificeEngine->getCameraPos(), artificeEngine->getCameraPos() + artificeEngine->getCameraFront(), artificeEngine->getCameraUp());
	artificeShaderProgram.setMat4("view", viewMatrix);
	//artificeShaderProgram.setMat4("view", artificeEngine->getViewMatrix());

	std::vector<model> modelsToRaster = artificeEngine->modelsToRaster;
	//artificeEngine->mtx.unlock();

	glBindVertexArray(gVAO);

	for (auto &model : modelsToRaster)
	{
		glm::mat4 modelMatrix = glm::mat4(1.0f); //make sure to initialize matrix to identity matrix first
		modelMatrix = glm::translate(modelMatrix, model.position);
		model.modelMatrix = modelMatrix;
		artificeShaderProgram.setMat4("model", model.modelMatrix);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	//glDrawElements( GL_TRIANGLES, indexDataSize, GL_UNSIGNED_INT, NULL );

	//unbind program - deactivate shader
	//artificeShaderProgram.unbind();
}

void close()
{
	//Deallocate program
	glDeleteProgram( gProgramID );

	//Destroy window	
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//start the 3D engine
		std::thread t = artificeEngine->startEngine();

		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		
		//Enable text input
		SDL_StartTextInput();

		//While application is running
		while( !quit )
		{

			eventController.clearMouseMotionState();

			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//user requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
					artificeEngine->isActive = false;
					isActive = false;
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
			}

			//Render
			render();

			//Update screen
			SDL_GL_SwapWindow( gWindow );
		}

		//Disable text input
		SDL_StopTextInput();
	}

	//Free resources and close SDL
	close();

	return 0;
}
