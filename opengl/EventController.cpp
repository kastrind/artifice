#include "EventController.h"

bool* EventController::getKeysPressed()
{
	return keysPressed;
}

int EventController::getMouseDistanceX()
{
	return mouseDistanceX;
}

int EventController::getMouseDistanceY()
{
	return mouseDistanceY;
}

void EventController::clearMouseMotionState ()
{
	keysPressed[SupportedKeys::MOUSE_UP] = false;
	keysPressed[SupportedKeys::MOUSE_DOWN] = false;
	keysPressed[SupportedKeys::MOUSE_LEFT] = false;
	keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
	mouseDistanceX = 0;
	mouseDistanceY = 0;
}

void EventController::processEvent(SDL_Event* e)
{
	if (e->type == SDL_MOUSEMOTION) {
		//get mouse position
		//SDL_GetMouseState( &mousePosX, &mousePosY );
		SDL_GetRelativeMouseState( &mousePosX, &mousePosY );
		mouseDistanceX = std::abs(mousePosX - prevMousePosX);
		mouseDistanceY = std::abs(mousePosY - prevMousePosY);
		if (mousePosX < prevMousePosX) {
			keysPressed[SupportedKeys::MOUSE_RIGHT] = true;
			keysPressed[SupportedKeys::MOUSE_LEFT] = false;
		}else if (mousePosX > prevMousePosX) {
			keysPressed[SupportedKeys::MOUSE_LEFT] = true;
			keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
		}else {
			keysPressed[SupportedKeys::MOUSE_LEFT] = false;
			keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
		}
		if (mousePosY < prevMousePosY) {
			keysPressed[SupportedKeys::MOUSE_DOWN] = true;
			keysPressed[SupportedKeys::MOUSE_UP] = false;
		}else if (mousePosY > prevMousePosY) {
			keysPressed[SupportedKeys::MOUSE_UP] = true;
			keysPressed[SupportedKeys::MOUSE_DOWN] = false;
		}else {
			keysPressed[SupportedKeys::MOUSE_UP] = false;
			keysPressed[SupportedKeys::MOUSE_DOWN] = false;
		}
		prevMousePosX = mousePosX;
		prevMousePosY = mousePosY;
	}

	//user presses a key
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
		else if (e->key.keysym.sym == SDLK_w)
		{
			keysPressed[SupportedKeys::W] = true;
		}
		else if (e->key.keysym.sym == SDLK_s)
		{
			keysPressed[SupportedKeys::S] = true;
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
	//user releases a key
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
		else if (e->key.keysym.sym == SDLK_w)
		{
			keysPressed[SupportedKeys::W] = false;
		}
		else if (e->key.keysym.sym == SDLK_s)
		{
			keysPressed[SupportedKeys::S] = false;
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