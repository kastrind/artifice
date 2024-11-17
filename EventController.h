#pragma once

//Using SDL and standard IO
#include "Configuration.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <queue>
#include <cstring>
#include <mutex>
#include <map>


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
	C,
	V,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_UP,
	MOUSE_DOWN,
	MOUSE_LEFT_CLICK,
	MOUSE_MIDDLE_CLICK,
	MOUSE_RIGHT_CLICK,
	MOUSE_WHEEL_UP,
	MOUSE_WHEEL_DOWN,
	LEFT_CTRL,
	LEFT_SHIFT,
	LEFT_ALT,
	SPACE,
	ALL_KEYS
} SupportedKeys;

typedef enum 
{
	ASCEND,
	DESCEND,
	LEFT,
	RIGHT,
	FORWARD,
	BACKWARD,
	COPY,
	PASTE,
	PLACE,
	REMOVE,
	NEXT,
	PREVIOUS,
	JUMP,
	ALL_ACTIONS
} KeyActions;

struct BoolArray {
	bool* array;
	size_t size;

	BoolArray() : array(nullptr), size(0) {}

	BoolArray(bool* arr, size_t s) {
		size = s;
		array = new bool[size];
		std::memcpy(array, arr, size * sizeof(bool));
	}

	BoolArray(const BoolArray& other) {
		size = other.size;
		array = new bool[size];
		std::memcpy(array, other.array, size * sizeof(bool));
	}

	BoolArray(BoolArray&& other) : array(other.array), size(other.size) {
		other.array = nullptr;
		other.size = 0;
	}

	BoolArray& operator = (BoolArray&& other) {
		if (this != &other) {
			delete[] array;
			array = other.array;
			size = other.size;
			other.array = nullptr;
			other.size = 0;
		}
		return *this;
	}

	~BoolArray() {
		if (array != nullptr) {
			delete[] array;
		}
	}

	BoolArray& operator=(const BoolArray& other) {
		if (this != &other) {
			delete[] array;
			size = other.size;
			array = new bool[size];
			std::memcpy(array, other.array, size * sizeof(bool));
		}
		return *this;
	}
};

class EventController
{
	public:

		EventController(float mouseSensitivityX = 1.0f, float mouseSensitivityY = 1.0f,
						std::string keyAscend = "UP_ARROW", std::string keyDescend = "DOWN_ARROW", std::string keyLeft = "A",
						std::string keyRight = "D", std::string keyForward = "W", std::string keyBackward = "S",
						std::string keyPlace = "MOUSE_LEFT", std::string keyRemove = "MOUSE_RIGHT", std::string keyNext = "MOUSE_WHEEL_UP",
						std::string keyPrevious = "MOUSE_WHEEL_DOWN", std::string keyJump = "SPACE")
						: mouseSensitivityX(mouseSensitivityX), mouseSensitivityY(mouseSensitivityY)
		{
			//initialize key state
			for (int i = 0; i < SupportedKeys::ALL_KEYS; i++)
			{
				keysPressed[i] = false;
			}

			//supported keys from string to enum
			supportedKeysFromStr["UP_ARROW"] = SupportedKeys::UP_ARROW;
			supportedKeysFromStr["DOWN_ARROW"] = SupportedKeys::DOWN_ARROW;
			supportedKeysFromStr["LEFT_ARROW"] = SupportedKeys::LEFT_ARROW;
			supportedKeysFromStr["RIGHT_ARROW"] = SupportedKeys::RIGHT_ARROW;
			supportedKeysFromStr["W"] = SupportedKeys::W;
			supportedKeysFromStr["S"] = SupportedKeys::S;
			supportedKeysFromStr["A"] = SupportedKeys::A;
			supportedKeysFromStr["D"] = SupportedKeys::D;
			supportedKeysFromStr["C"] = SupportedKeys::C;
			supportedKeysFromStr["V"] = SupportedKeys::V;
			supportedKeysFromStr["MOUSE_LEFT"] = SupportedKeys::MOUSE_LEFT;
			supportedKeysFromStr["MOUSE_RIGHT"] = SupportedKeys::MOUSE_RIGHT;
			supportedKeysFromStr["MOUSE_UP"] = SupportedKeys::MOUSE_UP;
			supportedKeysFromStr["MOUSE_DOWN"] = SupportedKeys::MOUSE_DOWN;
			supportedKeysFromStr["MOUSE_LEFT_CLICK"] = SupportedKeys::MOUSE_LEFT_CLICK;
			supportedKeysFromStr["MOUSE_MIDDLE_CLICK"] = SupportedKeys::MOUSE_MIDDLE_CLICK;
			supportedKeysFromStr["MOUSE_RIGHT_CLICK"] = SupportedKeys::MOUSE_RIGHT_CLICK;
			supportedKeysFromStr["MOUSE_WHEEL_UP"] = SupportedKeys::MOUSE_WHEEL_UP;
			supportedKeysFromStr["MOUSE_WHEEL_DOWN"] = SupportedKeys::MOUSE_WHEEL_DOWN;
			supportedKeysFromStr["LEFT_CTRL"] = SupportedKeys::LEFT_CTRL;
			supportedKeysFromStr["LEFT_SHIFT"] = SupportedKeys::LEFT_SHIFT;
			supportedKeysFromStr["LEFT_ALT"] = SupportedKeys::LEFT_ALT;
			supportedKeysFromStr["SPACE"] = SupportedKeys::SPACE;

			//actions from string to enum
			keyActionsFromStr["ASCEND"] = KeyActions::ASCEND;
			keyActionsFromStr["DESCEND"] = KeyActions::DESCEND;
			keyActionsFromStr["LEFT"] = KeyActions::LEFT;
			keyActionsFromStr["RIGHT"] = KeyActions::RIGHT;
			keyActionsFromStr["FORWARD"] = KeyActions::FORWARD;
			keyActionsFromStr["BACKWARD"] = KeyActions::BACKWARD;
			keyActionsFromStr["COPY"] = KeyActions::COPY;
			keyActionsFromStr["PASTE"] = KeyActions::PASTE;
			keyActionsFromStr["PLACE"] = KeyActions::PLACE;
			keyActionsFromStr["REMOVE"] = KeyActions::REMOVE;
			keyActionsFromStr["NEXT"] = KeyActions::NEXT;
			keyActionsFromStr["PREVIOUS"] = KeyActions::PREVIOUS;
			keyActionsFromStr["JUMP"] = KeyActions::JUMP;

			//map action to key
			mapActionToKey(KeyActions::ASCEND, keyAscend, SupportedKeys::UP_ARROW);
			mapActionToKey(KeyActions::DESCEND, keyDescend, SupportedKeys::DOWN_ARROW);
			mapActionToKey(KeyActions::LEFT, keyLeft, SupportedKeys::A);
			mapActionToKey(KeyActions::RIGHT, keyRight, SupportedKeys::D);
			mapActionToKey(KeyActions::FORWARD, keyForward, SupportedKeys::W);
			mapActionToKey(KeyActions::BACKWARD, keyBackward, SupportedKeys::S);
			mapActionToKey(KeyActions::PLACE, keyPlace, SupportedKeys::MOUSE_LEFT_CLICK);
			mapActionToKey(KeyActions::REMOVE, keyRemove, SupportedKeys::MOUSE_RIGHT_CLICK);
			mapActionToKey(KeyActions::NEXT, keyNext, SupportedKeys::MOUSE_WHEEL_UP);
			mapActionToKey(KeyActions::PREVIOUS, keyPrevious, SupportedKeys::MOUSE_WHEEL_DOWN);
			mapActionToKey(KeyActions::JUMP, keyJump, SupportedKeys::SPACE);
		}

		bool* getKeysPressed();

		void setMouseSensitivityX(float mouseSensitivityX);

		float getMouseSensitivityX();

		void setMouseSensitivityY(float mouseSensitivityY);

		float getMouseSensitivityY();

		int getMouseRelX();

		int getMouseRelY();

		int getMouseWheelY();

		void clearMouseMotionState();

		void decodeEvent(SDL_Event* e);

		bool ascend(bool* keysPressed);

		bool descend(bool* keysPressed);

		bool left(bool* keysPressed);

		bool right(bool* keysPressed);

		bool forward(bool* keysPressed);

		bool backward(bool* keysPressed);

		bool place(bool* keysPressed);

		bool remove(bool* keysPressed);

		bool next(bool* keysPressed, bool* prevKeysPressed);

		bool previous(bool* keysPressed, bool* prevKeysPressed);

		bool jump(bool* keysPressed);

		std::mutex mtx;

	private:

		void bufferKeysPressed();

		void mapActionToKey(KeyActions action, std::string keyStr, SupportedKeys defaultKey);

		std::map<std::string, SupportedKeys> supportedKeysFromStr;

		std::map<std::string, KeyActions> keyActionsFromStr;

		std::map<KeyActions, SupportedKeys> keyMappings;

		bool keysPressed[SupportedKeys::ALL_KEYS];

		bool poppedKeysPressed[SupportedKeys::ALL_KEYS];

		std::queue<BoolArray> keysPressedQueue;

		float mouseSensitivityX;
		float mouseSensitivityY;

		int mouseRelX;
		int mouseRelY;

		int mouseWheelY;
		int prevMouseWheelY;
};