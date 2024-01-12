#pragma once

#include "Configuration.h"
#include "EventController.h"
#include "Engine3D.h"
#include "Player.h"
#include "LTimer.h"
//Using SDL and standard IO
#include <SDL.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>

CFG cfg;

//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//the graphics engine
Engine3D* artificeEngine;

bool isActive=true;
void run();

bool init()
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
		std::string windowTitle = cfg.NAME + " v." + cfg.VERSION + '\0';
		gWindow = SDL_CreateWindow( windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
			//confine mouse cursor to the window and hide it
			SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
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

int main( int argc, char* args[] )
{
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		EventController eventController;
		std::string engineName = cfg.NAME + " v." + cfg.VERSION + '\0';
		artificeEngine = new Engine3D(engineName, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);
		std::thread t = artificeEngine->startEngine();
		std::thread t2 = std::thread(run);

		//the frames per second timer
		LTimer fpsTimer;
		//the frames per second cap timer
		LTimer capTimer;
		int frameTicks;
		float avgFPS;
		//start counting frames per second
		int countedFrames = 0;
		Player player(vec3d{100, 100, 0}, gRenderer, &eventController);
		//main loop flag
		bool quit = false;
		//event handler
		SDL_Event e;

		fpsTimer.start();

		//handle events on queue
		while( quit == false )
		{
			//start cap timer
			capTimer.start();

			eventController.clearMouseMotionState();

			//poll events
			while( SDL_PollEvent( &e ) )
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
				}else if (e.type == SDL_MOUSEBUTTONDOWN && SDL_GetWindowMouseGrab(gWindow) == SDL_FALSE) {
					//confine mouse cursor to the window and hide it
					SDL_SetWindowMouseGrab(gWindow, SDL_TRUE);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				//user presses or releases a key
				else if( e.type == SDL_KEYDOWN || e.type == SDL_KEYUP || e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP )
				{
					eventController.processEvent(&e);
				}
			}

			//calculate and correct fps
			avgFPS = countedFrames / ( fpsTimer.getTicks() / 1000.f );
			if( avgFPS > 2000000 )
			{
				avgFPS = 0;
			}

			std::cout << "avg fps:" << avgFPS << std::endl;

			//clear screen
			SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
			SDL_RenderClear( gRenderer );

			//move player
			player.move();

			//render player
			player.render();

			//std::cout << "triangles to raster: " << artificeEngine->trianglesToRaster.size() << std::endl;
			for (auto tri : artificeEngine->trianglesToRaster)
			{
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
				SDL_FPoint p1{tri.p[0].x, tri.p[0].y};
				SDL_FPoint p2{tri.p[1].x, tri.p[1].y};
				SDL_FPoint p3{tri.p[2].x, tri.p[2].y};
				SDL_FPoint points[4] = {p1, p2, p3, p1};
				SDL_RenderDrawLinesF(gRenderer, points, 4);
			}

			//update screen
			SDL_RenderPresent( gRenderer );

			++countedFrames;

			//if frame finished early
			frameTicks = capTimer.getTicks();
			if( frameTicks < cfg.SCREEN_TICK_PER_FRAME )
			{
				//wait remaining time
				SDL_Delay( cfg.SCREEN_TICK_PER_FRAME - frameTicks );
			}
		}
		t.join();
		t2.join();
	}
	return 0;
}



void run() {
	while(isActive) {
		//std::cout << "hello from thread 2" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
}
