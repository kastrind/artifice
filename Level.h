#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include "Constructs3D.h"

class Level
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 13;

		Level()
		{
		}

		std::vector<std::shared_ptr<model>> models;

		unsigned long modelPointsCnt = 0;

		unsigned long cubePointsCnt = 0;

		void load(std::string levelPath);

	private:

};