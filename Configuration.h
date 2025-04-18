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

	const std::string VERSION = "0.0.2";

	const std::string ASSETS_PATH = "assets";

	const std::string LEVELS_PATH = "levels";

	const std::string PATH_SEP = "/";

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

	float GRAVITATIONAL_PULL = 0.1f;

	float JUMP_SPEED_FACTOR = 5.0f;

	float PERSON_SPEED_FACTOR = 1.5f;

	float PERSON_HEIGHT = 0.15f; //vertical colliding distance

	float PERSON_WIDTH = 0.05f; //horizontal colliding distance

	float MOUSE_SENSITIVITY_X = 1.0f;

	float MOUSE_SENSITIVITY_Y = 1.0f;

	std::string KEY_ASCEND = "UP_ARROW";

	std::string KEY_DESCEND = "DOWN_ARROW";

	std::string KEY_LEFT = "A";

	std::string KEY_RIGHT = "D";

	std::string KEY_FORWARD = "W";

	std::string KEY_BACKWARD = "S";

	std::string KEY_PLACE = "MOUSE_LEFT_CLICK";

	std::string KEY_REMOVE = "MOUSE_RIGHT_CLICK";

	std::string KEY_NEXT = "MOUSE_WHEEL_UP";

	std::string KEY_PREVIOUS = "MOUSE_WHEEL_DOWN";

	std::string KEY_JUMP = "SPACE";

	//precomputed constants follow

	float FOV_RADIANS = (float) FOV * M_PI / 180;

	const float M_PI_HALF = M_PI / 2;

	const float M_PI_X_2 = 2 * M_PI;

	const float M_PI_X_3_HALF = 3 * M_PI / 2;

} CFG;

extern CFG cfg;
