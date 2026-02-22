#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <stdexcept>
#include "Constructs3D.h"
#include "Configuration.h"
#include "Level.h"

class Level;

class CompoundModel
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 18;

		CompoundModel(unsigned long id=0): id(id) {}

		CompoundModel(unsigned long modelPointsCntOffset, unsigned long cubePointsCntOffset, unsigned long id=0)
		: modelPointsCnt(modelPointsCntOffset), cubePointsCnt(cubePointsCntOffset), id(id) {}

		std::vector<std::shared_ptr<model>> models;

		std::string modelPath;

		unsigned long id = 0;

		unsigned long modelPointsCnt = 0;

		unsigned long cubePointsCnt = 0;

		void load(std::string modelPath, Transform* transform);

		static std::shared_ptr<CompoundModel> create(std::string modelPath, Transform* transform, unsigned long modelPointsCntOffset = 0.0f, unsigned long cubePointsCntOffset = 0.0f, unsigned long id=0);

		void save(std::string modelPath = "");

	private:

		// keeps track of loaded compound models to protect from circular dependencies
		static std::set<unsigned long> loadedIds;

		// registers loading compound models to protect from circular dependencies
		static bool registerLoadedId(unsigned long id);

		// un-registers loaded compound models to protect from circular dependencies
		static void unregisterLoadedId(unsigned long id);

};