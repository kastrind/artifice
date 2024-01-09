#pragma once

//Using SDL and standard IO
#include <SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "Configuration.h"
#include "Engine3D.h"
#include "EventController.h"

class Player
{
	public:

		static const int DOT_WIDTH = 8;
		static const int DOT_HEIGHT = 8;
		static const int MAX_VEL = 3;

		Player (vec3d pos, SDL_Renderer* gRenderer, EventController* eController)
		: gRenderer(gRenderer), eController(eController),
		  p(pos), d{3,0,0}, vel{0,0,0}, angle(0)
		{}

		void render();

		void move();

	private:

		float angle;

		vec3d p; //position

		vec3d d; //displacement

		vec3d vel; //velocity

		SDL_Renderer* gRenderer;

		EventController* eController;
};