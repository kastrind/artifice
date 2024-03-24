#pragma once

//Using SDL and standard IO
#include "Configuration.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <queue>
#include <cstring>
#include <thread>
#include <atomic>
#include <mutex>


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
	MOUSE_LEFT_CLICK,
	MOUSE_MIDDLE_CLICK,
	MOUSE_RIGHT_CLICK,
	MOUSE_WHEEL_UP,
	MOUSE_WHEEL_DOWN,
	LEFT_CTRL,
	ALL_KEYS
} SupportedKeys;

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

		EventController()
		{
			for (int i = 0; i < SupportedKeys::ALL_KEYS; i++)
			{
				keysPressed[i] = false;
			}
			isActive = false;
		}

		std::thread startListening();

		void pushEvent(SDL_Event e);

		bool* getKeysPressed();

		int getMouseDistanceX();

		int getMouseDistanceY();

		void clearMouseMotionState();

		std::mutex mtx;

		std::atomic<bool> isActive;

	private:

		void eventLoop();

		bool isMouseClicked();

		bool isMouseButton(SupportedKeys btn);

		void processEvent(SDL_Event* e);

		void bufferKeysPressed();

		bool keysPressed[SupportedKeys::ALL_KEYS];

		bool poppedKeysPressed[SupportedKeys::ALL_KEYS];

		std::queue<BoolArray> keysPressedQueue;

		std::queue<SDL_Event> sdlEventQueue;

		int mousePosX = 0;
		int prevMousePosX = 0;

		int mousePosY = 0;
		int prevMousePosY = 0;

		int mouseDistanceX = 0;
		int mouseDistanceY = 0;		
};