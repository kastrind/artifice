#pragma once

//Using SDL and standard IO
#include "Configuration.h"
#include <SDL.h>
#include <iostream>

typedef enum 
{
	UP_ARROW,
	DOWN_ARROW,
	LEFT_ARROW,
	RIGHT_ARROW,
	W,
	S,
	A,
	D,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_UP,
	MOUSE_DOWN,
	ALL_KEYS
} SupportedKeys;

class EventController
{
	public:

		EventController()
		{
			for (int i = 0; i < SupportedKeys::ALL_KEYS; i++)
			{
				keysPressed[i] = false;
			}
		}

		void processEvent(SDL_Event* e);

		void clearMouseMotionState();

		bool* getKeysPressed();

	private:

		bool keysPressed[SupportedKeys::ALL_KEYS];

		int mousePosX = 0;
		int prevMousePosX = 0;

		int mousePosY = 0;
		int prevMousePosY = 0;

};