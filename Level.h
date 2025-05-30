#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "Constructs3D.h"
#include "Light.h"

class Level
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 13;

		Level()
		{
			playerPosition = glm::vec3(0.0f, 0.0f, 3.0f);
		}

		std::vector<std::shared_ptr<model>> models;

		std::string levelPath;

		unsigned long modelPointsCnt = 0;

		unsigned long cubePointsCnt = 0;

		glm::vec3 playerPosition;

		Light light;

		PointLight pointLight;

		std::vector<PointLight> pointLights;

		void load(std::string levelPath);

		void save(std::string levelPath = "");

	private:

};