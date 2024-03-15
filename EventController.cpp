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

bool EventController::transitionedMouseLeftButton()
{
	return prevMouseLeftBtnPressed != keysPressed[SupportedKeys::MOUSE_LEFT_CLICK];
}

bool EventController::transitionedMouseMiddleButton()
{
	return !prevMouseMiddleBtnPressed && keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK];
}

bool EventController::transitionedMouseRightButton()
{
	return !prevMouseRightBtnPressed && keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK];
}

void EventController::clearMouseMotionState ()
{
	keysPressed[SupportedKeys::MOUSE_UP] = false;
	keysPressed[SupportedKeys::MOUSE_DOWN] = false;
	keysPressed[SupportedKeys::MOUSE_LEFT] = false;
	keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
	prevMouseLeftBtnPressed   = keysPressed[SupportedKeys::MOUSE_LEFT_CLICK];
	prevMouseMiddleBtnPressed = keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK];
	prevMouseRightBtnPressed  = keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK];
	mouseDistanceX = 0;
	mouseDistanceY = 0;
}

void EventController::processEvent(SDL_Event* e)
{
	unsigned short mouseBtnTest = SDL_BUTTON(SDL_GetMouseState(NULL, NULL));

	//User pressed mouse button
	if (e->type == SDL_MOUSEBUTTONDOWN)
	{
		switch(mouseBtnTest)
		{
			case 1:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = true;
			break;

			case 2:
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = true;
			break;

			case 4:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = true;
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = true;
			break;

			case 8:
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = true;
			break;

			case 16:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = true;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = true;
			break;

			case 32:
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = true;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = true;
			break;

			case 64:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = true;
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = true;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = true;
			break;			
		}
		//std::cout << "left: " << keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] << ", middle: " << keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] << ", right: " << keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] << std::endl;
	}

	//User released mouse button
	else if (e->type == SDL_MOUSEBUTTONUP)
	{
		switch(mouseBtnTest)
		{
			case 0:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = false;
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = false;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = false;
			break;

			case 1:
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = false;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = false;
			break;

			case 2:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = false;
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = false;
			break;

			case 4:
			keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] = false;
			break;

			case 8:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = false;
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = false;
			break;

			case 16:
			keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] = false;
			break;

			case 32:
			keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] = false;
			break;			
		}
		//std::cout << "left: " << keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] << ", middle: " << keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] << ", right: " << keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] << std::endl;
	}

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