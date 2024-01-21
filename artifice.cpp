#pragma once

#include "Configuration.h"
#include "EventController.h"
#include "Constructs3D.h"
#include "Engine3D.h"
#include "Player.h"
#include "LTimer.h"
//Using SDL and standard IO
#include <SDL.h>
#include <SDL_surface.h>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <stdio.h>

CFG cfg;

//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//window mouse barrier
SDL_Rect windowRect{0, 0, cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT};

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
			SDL_SetWindowMouseRect(gWindow, &windowRect);
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

		//load image at specified path
		SDL_Surface* loadedSurface = SDL_LoadBMP( "brickwall.bmp" );
		int imgWidth = loadedSurface->w;
		int imgHeight = loadedSurface->h;
		SDL_Texture* newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		SDL_FreeSurface(loadedSurface);

		//instantiate the 3D engine
		EventController eventController;
		artificeEngine = new Engine3D(cfg.SCREEN_WIDTH, cfg.SCREEN_HEIGHT, cfg.NEAR, cfg.FAR, cfg.FOV, &eventController);

		// //create a few models to send them to the engine for rendering
		// //create a rectangle
		// rectangle rect{3, 3, 3, 1,    2, 2,    0.3, 0, 0.3};
		// std::vector<triangle> rect_tris;
		// rect.toTriangles(rect_tris);
		// model model_rect;
		// model_rect.modelMesh.tris = rect_tris;
		// artificeEngine->modelsToRaster.push_back(model_rect);
		// //create a cuboid
		// cuboid box{0, 0, 0, 1,    1, 1, 1,    0.3, 0, 0.3};
		// std::vector<triangle> box_tris;
		// box.toTriangles(box_tris);
		// model model_box;
		// model_box.modelMesh.tris = box_tris;
		// artificeEngine->modelsToRaster.push_back(model_box);
		
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




		//start the 3D engine
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

			//calculate and correct fps
			avgFPS = countedFrames / ( fpsTimer.getTicks() / 1000.f );
			if( avgFPS > 2000000 )
			{
				avgFPS = 0;
			}

			//std::cout << "avg fps:" << avgFPS << std::endl;

			//clear screen
			SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0x00 );
			SDL_RenderClear( gRenderer );

			//move player
			//player.move();

			//render player
			//player.render();

			artificeEngine->mtx.lock();
			std::vector<triangle> trianglesToRender = artificeEngine->trianglesToRaster;
			artificeEngine->mtx.unlock();

			unsigned char r, g, b;
			std::vector<SDL_Vertex> verts;
			for (auto tri : trianglesToRender)
			{
				r = (float)tri.R * tri.luminance; g = (float)tri.G * tri.luminance; b = (float)tri.B * tri.luminance;
				// verts.push_back({ SDL_FPoint {tri.p[0].x, tri.p[0].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ 0 } });
				// verts.push_back({ SDL_FPoint {tri.p[1].x, tri.p[1].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ 0 } });
				// verts.push_back({ SDL_FPoint {tri.p[2].x, tri.p[2].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ 0 } });
				verts.push_back({ SDL_FPoint {tri.p[0].x, tri.p[0].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ tri.t[0].u/tri.t[0].w, tri.t[0].v/tri.t[0].w } });

				/*
				float slopeAB; float extraABx; float extraABy;
				if (tri.p[1].x != tri.p[0].x) {
					std::cout << "x0 " << tri.p[0].x << ", x1 " << tri.p[1].x << std::endl;
					std::cout << "y0 " << tri.p[0].y << ", y1 " << tri.p[1].y << std::endl;
					slopeAB = (float)(tri.p[1].y - tri.p[0].y) / (tri.p[1].x - tri.p[0].x);
					extraABx = tri.p[0].x < tri.p[1].x ?  tri.p[0].x + (tri.p[1].x - tri.p[0].x)/2 : tri.p[1].x + (tri.p[0].x - tri.p[1].x)/2;
					extraABy = slopeAB==0 ? tri.p[0].y : tri.p[0].y + slopeAB * (extraABx - tri.p[0].x);
					std::cout << "slope " << slopeAB << ", x " << extraABx << ", y " << extraABy << std::endl;
				}else {
					extraABx = tri.p[0].x;
					extraABy = tri.p[0].y < tri.p[1].y ?  tri.p[0].y + (tri.p[1].y - tri.p[0].y)/2 : tri.p[1].y + (tri.p[0].y - tri.p[1].y)/2;
					std::cout << extraABx << ", " << extraABy << std::endl;
				}
				verts.push_back({ SDL_FPoint {extraABx, extraABy}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ tri.t[1].u, tri.t[1].v } });


				float slopeAC; float extraACx; float extraACy;
				if (tri.p[2].x != tri.p[0].x) {
					std::cout << "x0 " << tri.p[0].x << ", x2 " << tri.p[2].x << std::endl;
					std::cout << "y0 " << tri.p[0].y << ", y2 " << tri.p[2].y << std::endl;
					slopeAC = (float)(tri.p[2].y - tri.p[0].y) / (tri.p[2].x - tri.p[0].x);
					extraACx = tri.p[0].x < tri.p[2].x ?  tri.p[0].x + (tri.p[2].x - tri.p[0].x)/2 : tri.p[2].x + (tri.p[0].x - tri.p[2].x)/2;
					extraACy = slopeAC==0 ? tri.p[0].y : tri.p[0].y + slopeAC * (extraACx - tri.p[0].x);
					std::cout << "slope " << slopeAC << ", x " << extraACx << ", y " << extraACy << std::endl;
				}else {
					extraACx = tri.p[0].x;
					extraACy = tri.p[0].y < tri.p[2].y ?  tri.p[0].y + (tri.p[2].y - tri.p[0].y)/2 : tri.p[2].y + (tri.p[0].y - tri.p[2].y)/2;
					std::cout << extraACx << ", " << extraACy << std::endl;
				}
				verts.push_back({ SDL_FPoint {extraACx, extraACy}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ tri.t[1].u, tri.t[1].v } });
				*/

				verts.push_back({ SDL_FPoint {tri.p[1].x, tri.p[1].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ tri.t[1].u/tri.t[1].w, tri.t[1].v/tri.t[1].w } });
				verts.push_back({ SDL_FPoint {tri.p[2].x, tri.p[2].y}, SDL_Color{ r, g, b, 255 }, SDL_FPoint{ tri.t[2].u/tri.t[2].w, tri.t[2].v/tri.t[2].w } });
				/*
					for (texturePoint& tp : tri.texturePoints)
					{
						// texture coordinates
						int u = (tp.t.u / tp.t.w) * imgWidth;
						int v = (tp.t.v / tp.t.w) * imgHeight;

						// screen coordinates
						int x = tp.p.u;
						int y = tp.p.v;

						SDL_Rect rectangleSrc{u, v, 1, 1};
						SDL_Rect rectangleDst{x, y, 1, 1};
						SDL_RenderCopy(gRenderer, newTexture, &rectangleSrc, &rectangleDst);
					}
				*/
					
			}
			SDL_RenderGeometry(gRenderer, newTexture, verts.data(), verts.size(), nullptr, 0);

			/*
			std::cout << "triangles to raster: " << artificeEngine->trianglesToRaster.size() << std::endl;
			for (auto tri : artificeEngine->trianglesToRaster)
			{
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
				SDL_FPoint p1{tri.p[0].x, tri.p[0].y};
				SDL_FPoint p2{tri.p[1].x, tri.p[1].y};
				SDL_FPoint p3{tri.p[2].x, tri.p[2].y};
				SDL_FPoint points[4] = {p1, p2, p3, p1};
				SDL_RenderDrawLinesF(gRenderer, points, 4);
			}
			*/

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
