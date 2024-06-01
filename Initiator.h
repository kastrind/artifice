#pragma once

//Using SDL
#include <SDL2/SDL.h>

#include <iostream>
#include <stdio.h>

#include "Configuration.h"


class Initiator
{
	public:

		Initiator(CFG& cfg) : cfg(cfg)
		{
			windowRect.x = cfg.SCREEN_WIDTH/4;
			windowRect.y = cfg.SCREEN_HEIGHT/4;
			windowRect.w = cfg.SCREEN_WIDTH/2;
			windowRect.h = cfg.SCREEN_HEIGHT/2;
		}

		bool initiated = false;

		//configuration
		CFG cfg;

		//the window we'll be rendering graphics to
		SDL_Window* gWindow = NULL;

		//window mouse barrier
		SDL_Rect windowRect;

		//overload function call operator
    	bool operator()() {
      		return init();
    	}

		//starts up SDL, creates window, and initializes OpenGL
		bool init();

		//frees media and shuts down SDL
		void close();

	private:

};