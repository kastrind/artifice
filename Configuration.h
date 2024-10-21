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

	UserMode USER_MODE = UserMode::EDITOR;

	unsigned int SCREEN_WIDTH = 800;

	unsigned int SCREEN_HEIGHT = 600;

	bool MSAA = true;

	unsigned int MSAA_SAMPLES = 8;

	bool FXAA = true;

	bool PHONG_LIGHTING = true;

	bool LIGHT_MAPPING = true;

	bool NORMAL_MAPPING = true;

	bool DISPLACEMENT_MAPPING = true;

	float NEAR = 0.01f;

	float FAR = 100.0f;

	unsigned int FOV = 45; //field of view in degrees

	float DOF = 20.0f; //depth of field

	float COLLIDING_DISTANCE = 0.2f;

	float GRAVITATIONAL_PULL = 0.1f;

	float JUMP_SPEED_FACTOR = 5.0f;

	float CAMERA_SPEED_FACTOR = 1.5f;

	//precomputed constants follow

	float FOV_RADIANS = (float) FOV * M_PI / 180;

	const float M_PI_HALF = M_PI / 2;

	const float M_PI_X_2 = 2 * M_PI;

	const float M_PI_X_3_HALF = 3 * M_PI / 2;

} CFG;

extern CFG cfg;
