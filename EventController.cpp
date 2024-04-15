#include "EventController.h"

bool* EventController::getKeysPressed()
{
	if (!keysPressedQueue.empty())
	{
		BoolArray ba = keysPressedQueue.front();
		for (size_t i = 0; i < ba.size; ++i)
		{
			if (isMouseButton((SupportedKeys)i))
				poppedKeysPressed[i] = keysPressed[i];	
			else poppedKeysPressed[i] = ba.array[i];
		}
		mtx.lock();
		keysPressedQueue.pop();
		mtx.unlock();
		return poppedKeysPressed;
	}
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

void EventController::clearMouseMotionState()
{
	keysPressed[SupportedKeys::MOUSE_UP] = false;
	keysPressed[SupportedKeys::MOUSE_DOWN] = false;
	keysPressed[SupportedKeys::MOUSE_LEFT] = false;
	keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
	keysPressed[SupportedKeys::MOUSE_WHEEL_UP] = false;
	keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] = false;
	mouseDistanceX = 0;
	mouseDistanceY = 0;
}

bool EventController::isMouseClicked() {
	return keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] || keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] || keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] ||
		   keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] || keysPressed[SupportedKeys::MOUSE_WHEEL_UP];
}

bool EventController::isMouseButton(SupportedKeys btn) {
	return btn == keysPressed[SupportedKeys::MOUSE_LEFT_CLICK] ||
		   btn == keysPressed[SupportedKeys::MOUSE_MIDDLE_CLICK] ||
		   btn == keysPressed[SupportedKeys::MOUSE_RIGHT_CLICK] ||
		   btn == keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] ||
		   btn == keysPressed[SupportedKeys::MOUSE_WHEEL_UP];
}

void EventController::decodeEvent(SDL_Event* e)
{
	unsigned short mouseBtnTest = SDL_BUTTON(SDL_GetMouseState(NULL, NULL));

	//user scrolls up or down
	if (e->type == SDL_MOUSEWHEEL)
	{
		if (e->wheel.y > 0)
		{
			keysPressed[SupportedKeys::MOUSE_WHEEL_UP] = true;
		}
		else if (e->wheel.y < 0)
		{
			keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] = true;
		}
	}
	else
	{
		keysPressed[SupportedKeys::MOUSE_WHEEL_UP] = false;
		keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] = false;
	}

	//user pressed mouse button
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

	//user released mouse button
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
		int tmpMouseDistanceX = std::abs(mousePosX - prevMousePosX);
		int tmpMouseDistanceY = std::abs(mousePosY - prevMousePosY);
		if (tmpMouseDistanceX > 0) {
			if (mousePosX < prevMousePosX) {
				keysPressed[SupportedKeys::MOUSE_RIGHT] = true;
				keysPressed[SupportedKeys::MOUSE_LEFT] = false;
			}else if (mousePosX > prevMousePosX) {
				keysPressed[SupportedKeys::MOUSE_LEFT] = true;
				keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
			}
			prevMousePosX = mousePosX;
			mouseDistanceX = tmpMouseDistanceX;
		}
		if (tmpMouseDistanceY > 0) {
			if (mousePosY < prevMousePosY) {
				keysPressed[SupportedKeys::MOUSE_DOWN] = true;
				keysPressed[SupportedKeys::MOUSE_UP] = false;
			}else if (mousePosY > prevMousePosY) {
				keysPressed[SupportedKeys::MOUSE_UP] = true;
				keysPressed[SupportedKeys::MOUSE_DOWN] = false;
			}
			prevMousePosY = mousePosY;
			mouseDistanceY = tmpMouseDistanceY;
		}
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
		else if (e->key.keysym.sym == SDLK_LCTRL)
		{
			keysPressed[SupportedKeys::LEFT_CTRL] = true;
		}
		else if (e->key.keysym.sym == SDLK_LSHIFT)
		{
			keysPressed[SupportedKeys::LEFT_SHIFT] = true;
		}
		else if (e->key.keysym.sym == SDLK_LALT)
		{
			keysPressed[SupportedKeys::LEFT_ALT] = true;
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
		else if (e->key.keysym.sym == SDLK_LCTRL)
		{
			keysPressed[SupportedKeys::LEFT_CTRL] = false;
		}
		else if (e->key.keysym.sym == SDLK_LSHIFT)
		{
			keysPressed[SupportedKeys::LEFT_SHIFT] = false;
		}
		else if (e->key.keysym.sym == SDLK_LALT)
		{
			keysPressed[SupportedKeys::LEFT_ALT] = false;
		}
	}
	//buffer mouse clicks
	if (isMouseClicked()) bufferKeysPressed();
}

void EventController::bufferKeysPressed()
{
	mtx.lock();
	keysPressedQueue.push(BoolArray(keysPressed, SupportedKeys::ALL_KEYS / sizeof(bool)));
	mtx.unlock();
}