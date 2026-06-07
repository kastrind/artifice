#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <stdexcept>
#include <cstdint>
#include "Constructs3D.h"
#include "Configuration.h"
#include "Level.h"

class Level;

class CompoundModel
{
	public:

		static const unsigned int NUM_ATTRIBUTES = 18;

		CompoundModel(uint64_t id=0): id(id) {}

		CompoundModel(uint64_t modelPointsCntOffset, uint64_t cubePointsCntOffset, uint64_t id=0)
		: modelPointsCnt(modelPointsCntOffset), cubePointsCnt(cubePointsCntOffset), id(id) {}

		std::vector<std::shared_ptr<model>> models;

		std::string modelPath;

		uint64_t id = 0;

		uint64_t modelPointsCnt = 0;

		uint64_t cubePointsCnt = 0;

		void load(std::string modelPath, Transform* transform);

		static std::shared_ptr<CompoundModel> create(std::string modelPath, Transform* transform, uint64_t modelPointsCntOffset = 0.0f, uint64_t cubePointsCntOffset = 0.0f, uint64_t id=0);

		void save(std::string modelPath = "");

	private:

		// keeps track of loaded compound models to protect from circular dependencies
		static std::set<uint64_t> loadedIds;

		// registers loading compound models to protect from circular dependencies
		static bool registerLoadedId(uint64_t id);

		// un-registers loaded compound models to protect from circular dependencies
		static void unregisterLoadedId(uint64_t id);

};