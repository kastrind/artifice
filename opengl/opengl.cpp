//Using SDL, SDL OpenGL, GLEW, standard IO, and strings
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <iostream>
#include <stdio.h>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();

//Renders to the screen
void render(size_t vertexDataSize, GLfloat* vertexData, size_t indexDataSize, GLuint* indexData);

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
SDL_Rect windowRect{0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT};

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
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
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

				//Initialize OpenGL
				if( !initGL() )
				{
					printf( "Unable to initialize OpenGL!\n" );
					success = false;
				}

				//instantiate the 3D engine
				artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);
				model box;
				box.modelMesh.tris = {

					// SOUTH
					{ 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

					// EAST           																			   
					{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

					// NORTH           																			   
					{ 1.0f, 0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

					// WEST            																			   
					{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

					// TOP             																			   
					{ 0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},

					// BOTTOM          																			  
					{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
					{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,}

				};

				artificeEngine->modelsToRaster.push_back(box);

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
		gProgramID = artificeShaderProgram.getProgramID();

		// //Get vertex attribute location
		// gVertexPos2DLocation = glGetAttribLocation( gProgramID, "LVertexPos2D" );
		// if( gVertexPos2DLocation == -1 )
		// {
		// 	printf( "LVertexPos2D is not a valid glsl program variable!\n" );
		// 	success = false;
		// }
		// else
		// {
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

		// }
	}
	return success;
}

void handleKeys( unsigned char key, int x, int y )
{
	//Toggle quad
	if( key == 'q' )
	{
		gRenderQuad = !gRenderQuad;
	}
}

void update()
{
	//No per frame update needed
}

void render(size_t vertexDataSize, GLfloat* vertexData, size_t indexDataSize, GLuint* indexData)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	
	//Render quad
	if( gRenderQuad )
	{

		//VBO data
		// GLfloat vertexData[] =
		// {
		// 	//positions          //colors            //texture coords
		// 	0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   //top right
		// 	0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   //bottom right
		// 	-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   //bottom left
		// 	-0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f    //top left 
		// };

		// GLuint indexData[] = {
		// 	0, 1, 2, //first triangle
		// 	3, 4, 5  //second triangle
		// };

		//std::cout << "index data size: " << indexDataSize << std::endl;

		glGenVertexArrays(1, &gVAO);
		glBindVertexArray(gVAO);

		//Create VBO
		glGenBuffers( 1, &gVBO );
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glBufferData( GL_ARRAY_BUFFER, vertexDataSize * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );
		//glBufferData( GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW );

		//Create IBO
		glGenBuffers( 1, &gIBO );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexDataSize * sizeof(GLuint), indexData, GL_STATIC_DRAW );

		//position attribute
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//texture coord attribute
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
		glEnableVertexAttribArray(2);

		//bind program - activate shader
		artificeShaderProgram.bind();
		glBindVertexArray(gVAO);

		glDrawElements( GL_TRIANGLES, indexDataSize, GL_UNSIGNED_INT, NULL );

		//unbind program - deactivate shader
		artificeShaderProgram.unbind();
	}
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
					SDL_SetRelativeMouseMode(SDL_FALSE);
					SDL_SetWindowMouseRect(gWindow, NULL);
				}else if (e.type == SDL_MOUSEBUTTONDOWN && SDL_GetWindowMouseGrab(gWindow) == SDL_FALSE) {
					//confine mouse cursor to the window and hide it
					SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
					SDL_SetWindowMouseRect(gWindow, &windowRect);
				}
				//user presses or releases a key
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
				{
					eventController.processEvent(&e);
				}
			}


			artificeEngine->mtx.lock();
			std::vector<triangle> trianglesToRaster = artificeEngine->trianglesToRaster;
			artificeEngine->mtx.unlock();

			std::vector<GLfloat> vertexData;
			std::vector<GLuint> indexData;
			GLuint indexCounter = 0;

			for (auto tri : trianglesToRaster)
			{
				//std::cout << "tri p0: " << tri.p[0].x << ", " << tri.p[0].y << ", " << tri.p[0].z << std::endl;

				//VBO data
				vertexData.push_back(tri.p[0].x);
				vertexData.push_back(tri.p[0].y);
				vertexData.push_back(tri.p[0].z);
				vertexData.push_back(tri.p[0].w);
				vertexData.push_back((float)tri.R/255);
				vertexData.push_back((float)tri.G/255);
				vertexData.push_back((float)tri.B/255);
				vertexData.push_back(tri.t[0].u);
				vertexData.push_back(tri.t[0].v);
				vertexData.push_back(tri.t[0].w);

				indexData.push_back(indexCounter++);

				vertexData.push_back(tri.p[1].x);
				vertexData.push_back(tri.p[1].y);
				vertexData.push_back(tri.p[1].z);
				vertexData.push_back(tri.p[1].w);
				vertexData.push_back((float)tri.R/255);
				vertexData.push_back((float)tri.G/255);
				vertexData.push_back((float)tri.B/255);
				vertexData.push_back(tri.t[1].u);
				vertexData.push_back(tri.t[1].v);
				vertexData.push_back(tri.t[1].w);

				indexData.push_back(indexCounter++);

				vertexData.push_back(tri.p[2].x);
				vertexData.push_back(tri.p[2].y);
				vertexData.push_back(tri.p[2].z);
				vertexData.push_back(tri.p[2].w);
				vertexData.push_back((float)tri.R/255);
				vertexData.push_back((float)tri.G/255);
				vertexData.push_back((float)tri.B/255);
				vertexData.push_back(tri.t[2].u);
				vertexData.push_back(tri.t[2].v);
				vertexData.push_back(tri.t[2].w);

				indexData.push_back(indexCounter++);
			}

			if (vertexData.size()>0)
			{
				//Render
				render(vertexData.size(), vertexData.data(), indexData.size(), indexData.data());
			}
			
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
