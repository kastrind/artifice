#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

typedef enum UserMode
{
	PLAYER,
	EDITOR
} UserMode;

typedef struct Configuration
{
	const std::string NAME = "Artifice";

	const std::string VERSION = "0.0.1";

	const std::string ASSETS_PATH = "assets";

	const std::string LEVELS_PATH = "levels";

	const UserMode USER_MODE = UserMode::EDITOR;
	
	//const UserMode USER_MODE = UserMode::PLAYER;

	const unsigned int SCREEN_WIDTH = 800;

	const unsigned int SCREEN_HEIGHT = 600;

	const float NEAR = 0.01f;

	const float FAR = 100.0f;

	const unsigned int FOV = 45; //field of view in degrees

	const float DOF = 20.0f; //depth of field

	const float COLLIDING_DISTANCE = 0.2f;

	const float GRAVITATIONAL_PULL = 0.1f;

	const float JUMP_SPEED_FACTOR = 5.0f;

	const float CAMERA_SPEED_FACTOR = 1.5f;

	//precomputed constants follow

	const float FOV_RADIANS = (float) FOV * M_PI / 180;

	const float M_PI_HALF = M_PI / 2;

	const float M_PI_X_2 = 2 * M_PI;

	const float M_PI_X_3_HALF = 3 * M_PI / 2;

} CFG;

extern CFG cfg;
