#pragma once

//Using SDL and standard IO
#include <SDL.h>
#include <iostream>

typedef enum 
{
	UP_ARROW,
	DOWN_ARROW,
	LEFT_ARROW,
	RIGHT_ARROW,
	A,
	D,
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

		bool* getKeysPressed();

	private:

		bool keysPressed[SupportedKeys::ALL_KEYS];

};