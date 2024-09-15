#include "Initiator.h"
#include <fstream>
#include <sstream>

bool Initiator::init()
{
	//initialization flag
	bool success = true;

	loadConfiguration();

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
		gWindow = SDL_CreateWindow( "Artifice Engine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cfg->SCREEN_WIDTH, cfg->SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
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
			//use Vsync
			printf( "Setting VSync...\n" );
			if( SDL_GL_SetSwapInterval( 1 ) < 0 )
			{
				printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
			}
		}
	}

	initiated = success;
	return success;
}

void Initiator::loadConfiguration()
{
	printf( "Loading Configuration...\n");

	std::ifstream f("configuration.ini");
	if (!f.is_open())
	{
		printf( "Failed to load configuration.ini!\n" );
		return;
	}

	while (!f.eof())
	{
		char line[256];
		f.getline(line, 256);

		std::stringstream s;
		std::string streamstring;
		
		s << line;
		s >> streamstring;

		//std::cout << " line: " << line << std::endl;

		if (streamstring.c_str()[0] == '#')
		{
			//std::cout << "ignoring comment " << streamstring.c_str() << std::endl;
			continue;
		}
		else
		{
			std::string linestring(line);
			std::istringstream ss(linestring);
			std::string token;
			unsigned int i = -1;
			std::string tokens[2];
			while(std::getline(ss, token, '=')) {
				if (i == 1) break;
				tokens[++i] = token;
			}
			if (i == 1) {
				if (tokens[0] == "USER_MODE") {
					cfg->USER_MODE = tokens[1] == "EDITOR" ? UserMode::EDITOR : UserMode::PLAYER;
					std::cout << "USER_MODE = " << cfg->USER_MODE << std::endl;
				}
			}
		}
	}
	f.close();
}

void Initiator::close()
{
	printf("Destroying SDL window...\n");

	//destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	printf("Quitting SDL subsystems...\n");

	//quit SDL subsystems
	SDL_Quit();
}