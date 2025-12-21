#include "EventController.h"

bool* EventController::getKeysPressed()
{
	if (!keysPressedQueue.empty())
	{
		BoolArray ba = keysPressedQueue.front();
		for (size_t i = 0; i < ba.size; ++i)
		{
			poppedKeysPressed[i] = ba.array[i];
		}
		mtx.lock();
		keysPressedQueue.pop();
		mtx.unlock();
		return poppedKeysPressed;
	}
	return keysPressed;
}

void EventController::setMouseSensitivityX(float mouseSensitivityX)
{
	this->mouseSensitivityX = mouseSensitivityX;
}

float EventController::getMouseSensitivityX()
{
	return mouseSensitivityX;
}

void EventController::setMouseSensitivityY(float mouseSensitivityY)
{
	this->mouseSensitivityY = mouseSensitivityY;
}

float EventController::getMouseSensitivityY()
{
	return mouseSensitivityY;
}

int EventController::getMouseRelX()
{
	return mouseRelX;
}

int EventController::getMouseRelY()
{
	return mouseRelY;
}

int EventController::getMouseWheelY()
{
	return mouseWheelY;
}

void EventController::clearMouseMotionState()
{
	keysPressed[SupportedKeys::MOUSE_UP] = false;
	keysPressed[SupportedKeys::MOUSE_DOWN] = false;
	keysPressed[SupportedKeys::MOUSE_LEFT] = false;
	keysPressed[SupportedKeys::MOUSE_RIGHT] = false;
	keysPressed[SupportedKeys::MOUSE_WHEEL_UP] = false;
	keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] = false;
	mouseRelX = 0;
	mouseRelY = 0;
	mouseWheelY = 0;
	prevMouseWheelY = 0;
}

void EventController::decodeEvent(SDL_Event* e)
{
	bool prevKeysPressed[SupportedKeys::ALL_KEYS];
	std::memcpy(prevKeysPressed, keysPressed, SupportedKeys::ALL_KEYS * sizeof(bool));

	unsigned short mouseBtnTest = SDL_BUTTON(SDL_GetMouseState(NULL, NULL));

	//user scrolls up or down
	if (e->type == SDL_MOUSEWHEEL)
	{
		if (e->wheel.y > 0)
		{
			mouseWheelY = e->wheel.y;
			keysPressed[SupportedKeys::MOUSE_WHEEL_UP] = mouseWheelY > prevMouseWheelY;
		}
		else if (e->wheel.y < 0)
		{
			mouseWheelY = e->wheel.y;
			keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] = mouseWheelY < prevMouseWheelY;
		}
		prevMouseWheelY = mouseWheelY;
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
		mouseRelX = e->motion.xrel;
		mouseRelY = e->motion.yrel;
		
		keysPressed[SupportedKeys::MOUSE_RIGHT] = mouseRelX > 0;
		keysPressed[SupportedKeys::MOUSE_LEFT] = mouseRelX < 0;

		keysPressed[SupportedKeys::MOUSE_UP] = mouseRelY < 0;
		keysPressed[SupportedKeys::MOUSE_DOWN] = mouseRelY > 0;
	}

	//user presses a key
	if( e->type == SDL_KEYDOWN )
	{
		for (std::map<SupportedKeys, SDL_KeyCode>::iterator itr = sdlKeyCodeMappings.begin(); itr != sdlKeyCodeMappings.end(); itr++)
		{
			if (itr->second == e->key.keysym.sym)
			{
				keysPressed[itr->first] = true;
				break;
			}
		}
	}
	//user releases a key
	else if( e->type == SDL_KEYUP )
	{
		for (std::map<SupportedKeys, SDL_KeyCode>::iterator itr = sdlKeyCodeMappings.begin(); itr != sdlKeyCodeMappings.end(); itr++)
		{
			if (itr->second == e->key.keysym.sym)
			{
				keysPressed[itr->first] = false;
				break;
			}
		}
	}

	//buffer keys on key state change (key press and key release - NOT mouse movements)
	for (size_t i = 0; i < SupportedKeys::ALL_KEYS; i++) {
		if (prevKeysPressed[(SupportedKeys)i] != keysPressed[(SupportedKeys)i] &&
			(SupportedKeys)i != SupportedKeys::MOUSE_LEFT &&
			(SupportedKeys)i != SupportedKeys::MOUSE_RIGHT &&
			(SupportedKeys)i != SupportedKeys::MOUSE_UP &&
			(SupportedKeys)i != SupportedKeys::MOUSE_DOWN
			) {
			bufferKeysPressed();
			break;
		}
	}
}

bool EventController::ascend(bool * keysPressed)
{
	return keysPressed[keyMappings[KeyActions::ASCEND]];
}

bool EventController::descend(bool * keysPressed)
{
	return keysPressed[keyMappings[KeyActions::DESCEND]];
}

bool EventController::left(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::LEFT]];
}

bool EventController::right(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::RIGHT]];
}

bool EventController::forward(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::FORWARD]];
}

bool EventController::backward(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::BACKWARD]];
}

bool EventController::flashlight(bool* keysPressed, bool* prevKeysPressed)
{
	return !prevKeysPressed[SupportedKeys::F] && keysPressed[keyMappings[KeyActions::FLASHLIGHT]];
}

bool EventController::place(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::PLACE]];
}

bool EventController::remove(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::REMOVE]];
}

bool EventController::next(bool* keysPressed, bool* prevKeysPressed)
{
	return keysPressed[keyMappings[KeyActions::NEXT]];
}

bool EventController::previous(bool* keysPressed, bool* prevKeysPressed)
{
	return keysPressed[keyMappings[KeyActions::PREVIOUS]];
}

bool EventController::jump(bool* keysPressed)
{
	return keysPressed[keyMappings[KeyActions::JUMP]];
}

bool EventController::scrollDown(bool* keysPressed, bool* prevKeysPressed) {
	return prevKeysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] && keysPressed[SupportedKeys::MOUSE_WHEEL_DOWN] == false;
}

bool EventController::scrollUp(bool* keysPressed, bool* prevKeysPressed) {
	return prevKeysPressed[SupportedKeys::MOUSE_WHEEL_UP]==true && keysPressed[SupportedKeys::MOUSE_WHEEL_UP]==false;
}



void EventController::bufferKeysPressed()
{
	mtx.lock();
	keysPressedQueue.push(BoolArray(keysPressed, SupportedKeys::ALL_KEYS / sizeof(bool)));
	mtx.unlock();
}

void EventController::mapActionToKey(KeyActions action, std::string keyStr, SupportedKeys defaultKey)
{
	std::map<std::string, SupportedKeys>::iterator it = supportedKeysFromStr.find(keyStr);
	keyMappings[action] = it != supportedKeysFromStr.end() ? it->second : defaultKey;
}