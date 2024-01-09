#pragma once

#include "Configuration.h"
#include "EventController.h"
#include "Player.h"
#include "LTimer.h"
//Using SDL and standard IO
#include <SDL.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>

CFG cfg;

//Screen dimension constants
const int SCREEN_WIDTH = cfg.SCREEN_WIDTH;
const int SCREEN_HEIGHT = cfg.SCREEN_HEIGHT;

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

bool init();

void close();

int main( int argc, char* args[] )
{
	//start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
				//The frames per second timer
		LTimer fpsTimer;

		//In memory text stream
		std::stringstream timeText;

		//Start counting frames per second
		int countedFrames = 0;
		fpsTimer.start();

		EventController eventController;
		Player player(vec3d{100,100,0}, gRenderer, &eventController);

		//main loop flag
		bool quit = false;
		//event handler
		SDL_Event e;
		//handle events on queue
		while( quit == false )
		{
			//poll events
			while( SDL_PollEvent( &e ) )
			{
				//user requests quit
				if( e.type == SDL_QUIT )
				{
					quit = true;
				}
				//user presses or releases a key
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP )
				{
					eventController.processEvent(&e);
				}
			}

			//calculate and correct fps
			float avgFPS = countedFrames / ( fpsTimer.getTicks() / 1000.f );
			if( avgFPS > 2000000 )
			{
				avgFPS = 0;
			}

			timeText.str( "" );
			timeText << "Average Frames Per Second " << avgFPS;

			std::cout << "avg fps:" << avgFPS << std::endl;

			//clear screen
			SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
			SDL_RenderClear( gRenderer );

			//move player
			player.move();

			//render player
			player.render();

			//update screen
			SDL_RenderPresent( gRenderer );

			++countedFrames;
		}
	}

	//free resources and close SDL
	close();

	return 0;
}

bool init ()
{
	bool success = true;

	//initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//create window
		gWindow = SDL_CreateWindow( "Artifice", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
		}
	}
	return success;
}

void close()
{
	//destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gRenderer = NULL;
	gWindow = NULL;
	//quit SDL subsystems
	SDL_Quit();
}
