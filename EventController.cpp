#include "EventController.h"

bool* EventController::getKeysPressed()
{
	return keysPressed;
}

void EventController::processEvent(SDL_Event* e)
{
	//User presses a key
	if( e->type == SDL_KEYDOWN )
	{
		if (e->key.keysym.sym == SDLK_UP)
		{
			keysPressed[SupportedKeys::UP_ARROW] = true;
		}
		else if (e->key.keysym.sym == SDLK_DOWN)
		{
			keysPressed[SupportedKeys::DOWN_ARROW] = true;
		}
		else if (e->key.keysym.sym == SDLK_LEFT)
		{
			keysPressed[SupportedKeys::LEFT_ARROW] = true;
		}
		else if (e->key.keysym.sym == SDLK_RIGHT)
		{
			keysPressed[SupportedKeys::RIGHT_ARROW] = true;
		}
		else if (e->key.keysym.sym == SDLK_a)
		{
			keysPressed[SupportedKeys::A] = true;
		}
		else if (e->key.keysym.sym == SDLK_d)
		{
			keysPressed[SupportedKeys::D] = true;
		}
	}
	//User releases a key
	else if( e->type == SDL_KEYUP )
	{
		if (e->key.keysym.sym == SDLK_UP)
		{
			keysPressed[SupportedKeys::UP_ARROW] = false;
		}
		else if (e->key.keysym.sym == SDLK_DOWN)
		{
			keysPressed[SupportedKeys::DOWN_ARROW] = false;
		}
		else if (e->key.keysym.sym == SDLK_LEFT)
		{
			keysPressed[SupportedKeys::LEFT_ARROW] = false;
		}
		else if (e->key.keysym.sym == SDLK_RIGHT)
		{
			keysPressed[SupportedKeys::RIGHT_ARROW] = false;
		}
		else if (e->key.keysym.sym == SDLK_a)
		{
			keysPressed[SupportedKeys::A] = false;
		}
		else if (e->key.keysym.sym == SDLK_d)
		{
			keysPressed[SupportedKeys::D] = false;
		}
	}
}