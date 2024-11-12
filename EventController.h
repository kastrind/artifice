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
	STRAFE_LEFT,
	STRAFE_RIGHT,
	FORWARD,
	BACKWARD,
	COPY,
	PASTE,
	PAN_LEFT,
	PAN_RIGHT,
	PAN_UP,
	PAN_DOWN,
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

		EventController(float mouseSensitivityX = 1.0f, float mouseSensitivityY = 1.0f)
						: mouseSensitivityX(mouseSensitivityX), mouseSensitivityY(mouseSensitivityY)
		{
			for (int i = 0; i < SupportedKeys::ALL_KEYS; i++)
			{
				keysPressed[i] = false;
			}
			//initialize default key mappings
			this->keyMappings[KeyActions::ASCEND] = SupportedKeys::UP_ARROW;
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

		std::mutex mtx;

	private:

		void bufferKeysPressed();

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