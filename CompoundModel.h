#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "Constructs3D.h"
#include "Level.h"

class Level;

class CompoundModel
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 18;

		CompoundModel()
		{
		}

		std::vector<std::shared_ptr<model>> models;

		std::string modelPath;

		void load(std::string modelPath, Level* level, Transform* transform = nullptr);

		void load(std::string modelPath, Transform* transform = nullptr);

		void save(std::string modelPath = "");

	private:

};