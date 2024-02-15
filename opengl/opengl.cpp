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
void loadTextures();

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
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;
GLuint gVAO = 0;

//shader program
ArtificeShaderProgram artificeShaderProgram;

//declare textures
std::vector<GLuint> textureIds;

std::vector<std::string> texturePaths;

//window mouse barrier
SDL_Rect windowRect{cfg.SCREEN_WIDTH/4, cfg.SCREEN_HEIGHT/4, cfg.SCREEN_WIDTH/2, cfg.SCREEN_HEIGHT/2};

//the graphics engine
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
			gContext = SDL_GL_CreateContext( gWindow );
			if( gContext == NULL )
			{
				printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//initialize GLEW
				glewExperimental = GL_TRUE; 
				GLenum glewError = glewInit();
				if( glewError != GLEW_OK )
				{
					printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
				}

				//use Vsync
				if( SDL_GL_SetSwapInterval( 1 ) < 0 )
				{
					printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
				}

				//instantiate the 3D engine
				artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);

				//initialize OpenGL
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
	//success flag
	bool success = true;

	if (!artificeShaderProgram.loadProgram())
	{
		printf( "Unable to load basic shader!\n" );
		success = false;
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS); 
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		gProgramID = artificeShaderProgram.getProgramID();

		// //create a rectangle
		// rectangle rect0{0, 0, 0, 1,    0.2, 0.5,    0.0, 2.0, 3.0};
		// model mdl0;
		// mdl0.id = 2;
		// mdl0.position = glm::vec3( 0.0f,  0.2f,  0.2f);
		// rect0.toTriangles(mdl0.modelMesh.tris);
		// artificeEngine->modelsToRaster.push_back(mdl0);

		// //create a rectangle
		// rectangle rect0b{0, 0, 0, 1,    0.5, 0.2,    0.0, 2.0, 0.0};
		// model mdl0b;
		// mdl0b.id = 3;
		// mdl0b.position = glm::vec3( 0.2f,  0.7f,  0.5f);
		// rect0b.toTriangles(mdl0b.modelMesh.tris);
		// artificeEngine->modelsToRaster.push_back(mdl0b);

		model mdl;
		mdl.id = 0;
		mdl.position = glm::vec3( 0.0f,  0.0f,  0.0f);
		//create a cuboid
		cuboid box{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box.toTriangles(mdl.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl);

		model mdl2;
		mdl2.id = 1;
		mdl2.position = glm::vec3( 0.0f,  0.0f,  0.2f);
		cuboid box2{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box2.toTriangles(mdl2.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl2);

		model mdl3;
		mdl3.position = glm::vec3( -0.2f,  0.2f,  0.0f);
		cuboid box3{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box3.toTriangles(mdl3.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl3);

		model mdl4;
		mdl4.position = glm::vec3( -0.2f,  0.2f, 0.2f);
		cuboid box4{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box4.toTriangles(mdl4.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl4);

		model mdl5;
		mdl5.position = glm::vec3( 0.0f,  0.0f, 0.4f);
		cuboid box5{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box5.toTriangles(mdl5.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl5);

		model mdl6;
		mdl6.position = glm::vec3( 0.2f,  0.2f, 0.0f);
		cuboid box6{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box6.toTriangles(mdl6.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl6);

		model mdl7;
		mdl7.position = glm::vec3( 0.2f,  0.2f, 0.2f);
		cuboid box7{0, 0, 0, 1,    0.2, 0.2, 0.2,    0.0, 0.0, 0.0};
		box7.toTriangles(mdl7.modelMesh.tris);
		artificeEngine->modelsToRaster.push_back(mdl7);
		
		//create VAO
		glGenVertexArrays(1, &gVAO);
		glBindVertexArray(gVAO);

		//create VBO
		glGenBuffers( 1, &gVBO );
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );

		//create IBO
		glGenBuffers( 1, &gIBO );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );

		//update buffers with the new vertices
		updateVertices();

		//initialize clear color
		glClearColor( 0.f, 0.f, 0.f, 1.f );

		//generates and binds textures
		loadTextures();
	}
	return success;
}

void loadTextures()
{
	texturePaths.push_back("brickwall.bmp");
	texturePaths.push_back("brickwallPainted.bmp");
	for (std::string texturePath : texturePaths)
	{
		textureIds.push_back(0);
		//declare texture
		glGenTextures(1, &textureIds.back());
		//bind texture
		glBindTexture(GL_TEXTURE_2D, textureIds.back());

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
	artificeShaderProgram.bind();
	//set the uniforms
	int cnt=0;
	for (GLuint& textureId : textureIds)
	{
		glUniform1i(glGetUniformLocation(artificeShaderProgram.getProgramID(), std::string("texture" + std::to_string(textureId)).c_str()), cnt++);
	}

}

void updateVertices()
{
		std::vector<GLfloat> vertexData;
		std::vector<GLuint> indexData;
		GLuint indexCounter = 0;

		//populate vertex vectors with triangle vertex information for each model
		for (auto &model : artificeEngine->modelsToRaster)
		{
			for (auto &tri : model.modelMesh.tris)
			{
				for (int i = 0; i < 3; i++)
				{
					vertexData.push_back(tri.p[i].x);
					vertexData.push_back(tri.p[i].y);
					vertexData.push_back(tri.p[i].z);
					vertexData.push_back((float)tri.R/255.0f);
					vertexData.push_back((float)tri.G/255.0f);
					vertexData.push_back((float)tri.B/255.0f);
					vertexData.push_back(tri.t[i].u);
					vertexData.push_back(tri.t[i].v);
					indexData.push_back(indexCounter++);
				}
			}
		}

		std::cout << "vertex data size: " << vertexData.size() << std::endl;
		std::cout << "index data size: " << indexData.size() << std::endl;

		//update VBO
		glBufferData( GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW );

		//update IBO
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(GLuint), indexData.data(), GL_STATIC_DRAW );

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		// //texture id attribute
		// glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(8 * sizeof(float)));
		// glEnableVertexAttribArray(3);
}

void render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureIds.front());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureIds.back());

	artificeShaderProgram.setMat4("projection", artificeEngine->getProjectionMatrix());	
	artificeShaderProgram.setMat4("view", artificeEngine->getViewMatrix());

	glBindVertexArray(gVAO);
	unsigned int modelCnt = 0;
	unsigned int prevModelTrisSize = 0;
	for (auto &model : artificeEngine->modelsToRaster)
	{
		artificeShaderProgram.setMat4("model", model.modelMatrix);
		glDrawElements(GL_TRIANGLES, model.modelMesh.tris.size() * 3, GL_UNSIGNED_INT, (void*)(((modelCnt++) * (prevModelTrisSize * 3) ) * sizeof(float)));
		prevModelTrisSize = model.modelMesh.tris.size();
		if (model.inFocus)
		{
			std::cout << "in focus!" << std::endl;
		}
	}
}

void close()
{
	//unbind program - deactivate shader
	artificeShaderProgram.unbind();

	//deallocate program
	glDeleteProgram( gProgramID );

	//destroy window	
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;

	engineThread.join();

	//quit SDL subsystems
	SDL_Quit();
}

int main( int argc, char* args[] )
{
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
