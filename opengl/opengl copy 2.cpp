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

//Renders quad to the screen
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
			}
		}
	}

	return success;
}

bool initGL()
{
	//Success flag
	bool success = true;

	LArtificeShaderProgram artificeShaderProgram;
	if (!artificeShaderProgram.loadProgram())
	{
		printf( "Unable to load basic shader!\n" );
		success = false;
	}
	else
	{
		gProgramID = artificeShaderProgram.getProgramID();

		//Get vertex attribute location
		gVertexPos2DLocation = glGetAttribLocation( gProgramID, "LVertexPos2D" );
		if( gVertexPos2DLocation == -1 )
		{
			printf( "LVertexPos2D is not a valid glsl program variable!\n" );
			success = false;
		}
		else
		{
			//Initialize clear color
			glClearColor( 0.f, 0.f, 0.f, 1.f );

			//VBO data
			GLfloat vertexData[] =
			{
				// -0.5f, -0.5f,
				// 0.5f, -0.5f,
				// 0.5f,  0.5f,
				// -0.5f,  0.5f
				// positions        // colors            // texture coords
				0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
				0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
				-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,  // bottom left
				-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f   // top left 
			};

			//IBO data
			GLuint indexData[] = { 0, 1, 2, 3 };

			//Create VBO
			glGenBuffers( 1, &gVBO );
			glBindBuffer( GL_ARRAY_BUFFER, gVBO );
			glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );

			//Create IBO
			glGenBuffers( 1, &gIBO );
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW );


			//TODO: relocate the below
			//declare texture
			unsigned int texture;
			glGenTextures(1, &texture); 
			//bind texture
			glBindTexture(GL_TEXTURE_2D, texture);
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

		}
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

void render()
{
	//Clear color buffer
	glClear( GL_COLOR_BUFFER_BIT );
	
	//Render quad
	if( gRenderQuad )
	{
		//Bind program
		glUseProgram( gProgramID );

		//Enable vertex position
		glEnableVertexAttribArray( gVertexPos2DLocation );

		//VBO data
		GLfloat vertexData[] =
		{
			// -0.1f, -0.1f,
			// 0.1f, -0.1f,
			// 0.1f,  0.1f,
			// -0.1f,  0.1f,

			0.0f, 0.0f,
			0.1f, 0.0f,
			0.2f, 0.1f,
			0.3f, 0.4f,
			0.4f, 0.5f,
			-0.3f, 0.2f
		};

		//Set vertex data
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glBufferData( GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), vertexData, GL_STATIC_DRAW );

		glVertexAttribPointer( gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL );

		//IBO data
		GLuint indexData[] = { 0, 1, 2, 3, 4, 5};

		//Set index data and render
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indexData, GL_STATIC_DRAW );

		glDrawElements( GL_TRIANGLE_FAN, 6, GL_UNSIGNED_INT, NULL );

		//Disable vertex position
		glDisableVertexAttribArray( gVertexPos2DLocation );

		//Unbind program
		glUseProgram( NULL );
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
		//Main loop flag
		bool quit = false;

		//Event handler
		SDL_Event e;
		
		//Enable text input
		SDL_StartTextInput();

		//While application is running
		while( !quit )
		{
			//Handle events on queue
			while( SDL_PollEvent( &e ) != 0 )
			{
				//User requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				//Handle keypress with current mouse position
				else if( e.type == SDL_TEXTINPUT )
				{
					int x = 0, y = 0;
					SDL_GetMouseState( &x, &y );
					handleKeys( e.text.text[ 0 ], x, y );
				}
			}

			//Render quad
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
