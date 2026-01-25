#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "Constructs3D.h"

class CompoundModel
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 18;

		CompoundModel()
		{
		}

		std::vector<std::shared_ptr<model>> models;

		std::string modelPath;

		void load(std::string modelPath);

		void save(std::string modelPath = "");

	private:

};