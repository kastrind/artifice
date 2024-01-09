#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

typedef struct Configuration
{
	const int SCREEN_WIDTH = 800;

	const int SCREEN_HEIGHT = 600;

	const int DOF = 8; //depth of field

	const int FOV = 60; //field of view in degrees

	const int PIXEL_SIZE = 8;

	//precomputed constants follow

	const float FOV_RADIANS = (float) FOV * M_PI / 180;

	const float M_PI_HALF = M_PI / 2;

	const float M_PI_X_2 = 2 * M_PI;

	const float M_PI_X_3_HALF = 3 * M_PI / 2;

} CFG;

extern CFG cfg;
