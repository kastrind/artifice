#include "Level.h"
#include <fstream>
#include <sstream>

void Level::save(std::string levelPath)
{
	if (levelPath.empty()) {
		levelPath = this->levelPath;
	}
	printf( "Saving level %s\n",  levelPath.c_str() );

	std::ofstream f(levelPath);
	if (!f.is_open())
	{
		printf( "Failed to save level!\n" );
		return;
	}
	else
	{
		f << "# id shape texture width height depth isSolid rotationX rotationY rotationZ positionX positionY positionZ" << std::endl;

		for (auto &ptrModel : models)
		{
			model& m = *ptrModel;

			f << m.id << ",";

			if (m.modelMesh.shape == shapetype::CUBE)
			{
				cubeModel& cm = dynamic_cast<cubeModel &>(m);
				float size = cm.modelMesh.tris[0].p[0].x - cm.modelMesh.tris[2].p[0].x;
				f << ((cm.isSkyBox) ? "skybox" : "cube") << "," << cm.texture << "," << size << "," << size << "," << size;
			}
			else if (m.modelMesh.shape == shapetype::CUBOID)
			{
				float width = m.modelMesh.tris[0].p[0].x - m.modelMesh.tris[1].p[0].x;
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[1].y;
				float depth = m.modelMesh.tris[0].p[0].z - m.modelMesh.tris[2].p[0].z;
				f << "cuboid," << m.texture << "," << width << "," << height << "," << depth;

			}
			else if (m.modelMesh.shape == shapetype::RECTANGLE)
			{
				float width = m.modelMesh.tris[0].p[0].x - m.modelMesh.tris[1].p[0].x;
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[1].y;
				f << "rectangle," << m.texture << "," << width << "," << height << "," <<  "0";
			}
			float thetaRotX = atan2(-m.rotationMatrix[2][1], m.rotationMatrix[2][2]);
			float thetaRotY = atan2(m.rotationMatrix[2][0], sqrt(m.rotationMatrix[2][1] * m.rotationMatrix[2][1] + m.rotationMatrix[2][2] * m.rotationMatrix[2][2]));
			float thetaRotZ = atan2(-m.rotationMatrix[1][0], m.rotationMatrix[0][0]);
			f << "," << (m.isSolid ? "true" : "false") << "," << thetaRotX << "," << thetaRotY << "," << thetaRotZ << "," << m.position.x << "," << m.position.y << "," << m.position.z << std::endl;
		}
	}
	f.close();
}

void Level::load(std::string levelPath)
{
	this->levelPath = levelPath;
	printf( "Loading level %s\n",  levelPath.c_str() );

	std::ifstream f(levelPath);
	if (!f.is_open())
	{
		printf( "Failed to load level!\n" );
		return;
	}

	bool existsSkyBox = false;

	while (!f.eof())
	{
		char line[256];
		f.getline(line, 256);

		std::stringstream s;
		std::string streamstring;
		
		s << line;
		s >> streamstring;

		unsigned long id;
		char* idEndPtr;

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
				id      = std::strtoul(tokens[0].c_str(), &idEndPtr, 0);
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

				if (shape == "rectangle")
				{
					rectangle rectangle(width, height, rotationX, rotationY, rotationZ);
					model m(id, modelPointsCnt, texture, glm::vec3(positionX, positionY, positionZ), rectangle, isSolid);
					modelPointsCnt += m.modelMesh.tris.size() * 3;
					models.push_back(std::make_shared<model>(m));
				}
				else if (shape == "cuboid")
				{
					cuboid cuboid(width, height, depth, rotationX, rotationY, rotationZ);
					model m(id, modelPointsCnt, texture, glm::vec3(positionX, positionY, positionZ), cuboid, isSolid);
					modelPointsCnt += m.modelMesh.tris.size() * 3;
					models.push_back(std::make_shared<model>(m));
				}
				else if (shape == "cube" || shape == "skyBox" || shape == "skybox")
				{
					cube cube(std::max(width, std::max(height, depth)), rotationX, rotationY, rotationZ);
					cubeModel cubeMdl(id, cubePointsCnt, texture, glm::vec3(positionX, positionY, positionZ), cube, isSolid);
					cubeMdl.isSkyBox = (shape == "skyBox" || shape == "skybox") == true;
					cubeMdl.isActiveSkyBox = (!existsSkyBox && cubeMdl.isSkyBox); // only the first skyBox is active
					existsSkyBox = existsSkyBox || cubeMdl.isSkyBox;
					cubePointsCnt += cubeMdl.modelMesh.tris.size() * 3;
					models.push_back(std::make_shared<cubeModel>(cubeMdl));
				}
			}
		}
	}
}