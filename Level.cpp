#include "Level.h"
#include <fstream>
#include <strstream>

void Level::load(std::string levelPath)
{
	printf( "Loading level %s\n",  levelPath.c_str() );

	std::ifstream f(levelPath);
	if (!f.is_open())
	{
		printf( "Failed to load level!\n" );
		return;
	}

	while (!f.eof())
	{
		char line[256];
		f.getline(line, 256);

		std::stringstream s;
    	std::string streamstring;
		
		s << line;
		s >> streamstring;

		unsigned long id;
        std::string shape, texture;
		bool isSolid;
	    float width, height, depth, rotationX, rotationY, rotationZ, positionX, positionY, positionZ;

		std::cout << " line: " << line << std::endl;

		if (streamstring.c_str()[0] == '#')
		{
			std::cout << "ignoring comment " << streamstring.c_str() << std::endl;
			continue;
		}
		else
		{
			std::string linestring(line);
			std::istringstream ss(linestring);
			std::string token;
			unsigned int i = -1;
			std::string tokens[NUM_ATTRIBUTES];
			while(std::getline(ss, token, ',')) {
				if (i == NUM_ATTRIBUTES - 1) break;
				tokens[++i] = token;
			}
			if (i == NUM_ATTRIBUTES - 1) {
				id      = std::stol(tokens[0]);
				shape   = tokens[1];
				texture = tokens[2];
				width   = std::stof(tokens[3]);
				height  = std::stof(tokens[4]);
				depth   = std::stof(tokens[5]);
				isSolid = tokens[6] == "true";
				rotationX = std::stof(tokens[7]);
				rotationY = std::stof(tokens[8]);
				rotationZ = std::stof(tokens[9]);
				positionX = std::stof(tokens[10]);
				positionY = std::stof(tokens[11]);
				positionZ = std::stof(tokens[12]);
				//std::cout << "shape: " << shape << ", tex: " << texture << ", w: " << width << ", h: " << height << ", d: " << depth << ", rX: " << rotationX << ", rY: " << rotationY << ", rZ: " << rotationZ << ", pX: " << positionX << ", pY: " << positionY << ", pZ: " << positionZ << std::endl;

				model mdl;
				mdl.id = id;
				mdl.isSolid = isSolid;
				mdl.texture = texture;
				mdl.position = glm::vec3(positionX, positionY, positionZ);
				bool isShapeValid = false;

                if (shape == "rectangle")
				{
					rectangle rect{width, height};
					rect.thetaRotX = rotationX; rect.thetaRotY = rotationY; rect.thetaRotZ = rotationZ;
					rect.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::RECTANGLE;
					mdl.sn = modelPointsCnt; 
					modelPointsCnt += mdl.modelMesh.tris.size() * 3;
					isShapeValid = true;
				}
				else if (shape == "cuboid")
				{
					cuboid cuboid{width, height, depth};
					cuboid.thetaRotX = rotationX; cuboid.thetaRotY = rotationY; cuboid.thetaRotZ = rotationZ;
					cuboid.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::CUBOID;
					mdl.sn = modelPointsCnt; 
					modelPointsCnt += mdl.modelMesh.tris.size() * 3;
					isShapeValid = true;
				}
				else if (shape == "cube")
				{
					cube cube{std::max(width, std::max(height, depth))};
					cube.thetaRotX = rotationX; cube.thetaRotY = rotationY; cube.thetaRotZ = rotationZ;
					cube.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::CUBE;
					mdl.sn = cubePointsCnt;
					cubePointsCnt += mdl.modelMesh.tris.size() * 3;
					isShapeValid = true;
				}

				if (isShapeValid)
				{
					models.push_back(mdl);
				}
			}
		}
	}
}