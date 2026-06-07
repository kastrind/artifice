#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "Constructs3D.h"
#include "Light.h"
#include "Utility.h"
#include "CompoundModel.h"

class CompoundModel;

class Level
{
	private:

		class metadata {
			public:
				uint64_t compoundModelId = 0;
		};

	public:

		static const unsigned int NUM_ATTRIBUTES = 18;

		Level()
		{
			playerPosition = glm::vec3(0.0f, 0.0f, 3.0f);
		}

		std::vector<std::shared_ptr<model>> models;

		std::string levelPath;

		uint64_t modelPointsCnt = 0;

		uint64_t cubePointsCnt = 0;

		glm::vec3 playerPosition;

		Light light;

		std::vector<PointLight> pointLights;

		std::vector<SpotLight> spotLights;

		SpotLight flashLight;

		bool assignedFlashLight = false;

		metadata meta;

		void save(std::string levelPath = "");

		void load(std::string levelPath);

		void deserializeModels(std::ifstream& f, std::vector<std::shared_ptr<model>>& models);

};