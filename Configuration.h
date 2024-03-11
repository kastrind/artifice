#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

typedef struct Configuration
{
	const std::string NAME = "Artifice";

	const std::string VERSION = "0.0.1";

	const unsigned int SCREEN_WIDTH = 800;

	const unsigned int SCREEN_HEIGHT = 600;

	const float NEAR = 0.01f;

	const float FAR = 100.0f;

	const unsigned int FOV = 45; //field of view in degrees

	const float DOF = 10.0f; //depth of field

	const float COLLIDING_DISTANCE = 0.2f;

	const float GRAVITATIONAL_PULL = 0.1f;

	//precomputed constants follow

	const float FOV_RADIANS = (float) FOV * M_PI / 180;

	const float M_PI_HALF = M_PI / 2;

	const float M_PI_X_2 = 2 * M_PI;

	const float M_PI_X_3_HALF = 3 * M_PI / 2;

} CFG;

extern CFG cfg;
