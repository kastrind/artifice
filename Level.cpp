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

    std::stringstream s;
    std::string streamstring;

	while (!f.eof())
	{
		char line[256];
		f.getline(line, 256);
		
		s << line;
		s >> streamstring;

        std::string shape, texture;
	    float width, height, depth, rotationX, rotationY, rotationZ, positionX, positionY, positionZ;

		if (streamstring.c_str()[0] == '#')
		{
			//std::cout << "ignoring comment" << std::endl;
			continue;
		}
		else
		{
			std::string linestring(line);
			std::istringstream ss(linestring);
			std::string token;
			unsigned int i = -1;
			std::string tokens[11];
			while(std::getline(ss, token, ',')) {
				if (i == 10) break;
				tokens[++i] = token;
			}
			if (i == 10) {
				shape   = tokens[0];
				texture = tokens[1];
				width   = std::stof(tokens[2]);
				height  = std::stof(tokens[3]);
				depth   = std::stof(tokens[4]);
				rotationX = std::stof(tokens[5]);
				rotationY = std::stof(tokens[6]);
				rotationZ = std::stof(tokens[7]);
				positionX = std::stof(tokens[8]);
				positionY = std::stof(tokens[9]);
				positionZ = std::stof(tokens[10]);
				//std::cout << "shape: " << shape << ", tex: " << texture << ", w: " << width << ", h: " << height << ", d: " << depth << ", rX: " << rotationX << ", rY: " << rotationY << ", rZ: " << rotationZ << ", pX: " << positionX << ", pY: " << positionY << ", pZ: " << positionZ << std::endl;

				model mdl;
				mdl.texture = texture;
				mdl.position = glm::vec3(positionX, positionY, positionZ);
				bool isShapeValid = false;

                if (shape == "rectangle")
				{
					rectangle rect{width, height};
					rect.thetaRotX = rotationX; rect.thetaRotY = rotationY; rect.thetaRotZ = rotationZ;
					rect.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::RECTANGLE;
					isShapeValid = true;
				}
				else if (shape == "cuboid")
				{
					cuboid cuboid{width, height, depth};
					cuboid.thetaRotX = rotationX; cuboid.thetaRotY = rotationY; cuboid.thetaRotZ = rotationZ;
					cuboid.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::CUBOID;
					isShapeValid = true;
				}
				else if (shape == "cube")
				{
					cube cube{std::max(width, std::max(height, depth))};
					cube.thetaRotX = rotationX; cube.thetaRotY = rotationY; cube.thetaRotZ = rotationZ;
					cube.toTriangles(mdl.modelMesh.tris);
					mdl.modelMesh.shape = Shape::CUBE;
					isShapeValid = true;
				}

				if (isShapeValid) { models.push_back(mdl); }
			}
		}
	}
}