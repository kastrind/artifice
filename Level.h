#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Constructs3D.h"

class Level
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 13;

		Level()
		{
		}

        std::vector<model> models;

        void load(std::string levelPath);

	private:

};