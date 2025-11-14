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
		f << "# player position positionX positionY positionZ" << std::endl;
		f << "player_position," << playerPosition.x << "," << playerPosition.y << "," << playerPosition.z << std::endl;
		f << "# light positionX positionY positionZ colorR colorG colorB ambientIntensity diffuseIntensity specularIntensity" << std::endl;
		f << "light," << light.direction.x << "," << light.direction.y << "," << light.direction.z << "," << light.color.r * 255 << "," << light.color.g * 255 << "," << light.color.b * 255 << "," << light.ambientIntensity << "," << light.diffuseIntensity << "," << light.specularIntensity << std::endl;

		f << "# id point light positionX positionY positionZ colorR colorG colorB ambientIntensity diffuseIntensity specularIntensity constant linear quadratic" << std::endl;
		for (PointLight& pl : pointLights)
		{
			// point lights without id are ignored, like the unlit one which is added if no point lights exist
			if (pl.id == 0) {
				continue;
			}
			f << pl.id << "," << "point_light," << pl.position.x << "," << pl.position.y << "," << pl.position.z << "," << pl.color.r * 255 << "," << pl.color.g * 255 << "," << pl.color.b * 255 << "," << pl.diffuseIntensity << "," << pl.specularIntensity << "," << pl.constant << "," << pl.linear << "," << pl.quadratic << std::endl;
		}
		f << "# id spot light positionX positionY positionZ colorR colorG colorB ambientIntensity diffuseIntensity specularIntensity constant linear quadratic dirX dirY dirZ cutoff outerCutoff" << std::endl;
		for (SpotLight& sl : spotLights)
		{
			// spot lights without id are ignored, like the unlit one which is added if no spot lights exist
			if (sl.id == 0) {
				continue;
			}
			f << sl.id << "," << "spot_light," << sl.position.x << "," << sl.position.y << "," << sl.position.z << "," << sl.color.r * 255 << "," << sl.color.g * 255 << "," << sl.color.b * 255 << "," << sl.diffuseIntensity << "," << sl.specularIntensity << "," << sl.constant << "," << sl.linear << "," << sl.quadratic << "," << sl.direction.x << "," << sl.direction.y << "," << sl.direction.z << "," << sl.cutoff << "," << sl.outerCutoff << std::endl;
		}

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
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[2].y;
				float depth = m.modelMesh.tris[0].p[0].z - m.modelMesh.tris[2].p[0].z;
				f << "cuboid," << m.texture << "," << width << "," << height << "," << depth;

			}
			else if (m.modelMesh.shape == shapetype::RECTANGLE)
			{
				float width = m.modelMesh.tris[0].p[1].x - m.modelMesh.tris[0].p[0].x;
				float height = m.modelMesh.tris[0].p[0].y - m.modelMesh.tris[0].p[2].y;
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

		//std::cout << " line: " << line << std::endl;

		if (streamstring.c_str()[0] == '#')
		{
			//std::cout << "ignoring comment " << streamstring.c_str() << std::endl;
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
				if (i == NUM_ATTRIBUTES - 1 || (i == 3 && tokens[0] == "player_position")) break;
				tokens[++i] = token;
			}
			if (i == NUM_ATTRIBUTES - 1 && (tokens[1] == "rectangle" || tokens[1] == "cuboid" || tokens[1] == "cube" || tokens[1] == "skyBox" || tokens[1] == "skybox")) {
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
			}else if (tokens[0] == "player_position") {
				positionX = std::stof(tokens[1]);
				positionY = std::stof(tokens[2]);
				positionZ = std::stof(tokens[3]);
				playerPosition = glm::vec3(positionX, positionY, positionZ);
			}else if (tokens[0] == "light") {
				light.direction = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
				light.color = glm::vec3(std::stoi(tokens[4])/255.0f, std::stoi(tokens[5])/255.0f, std::stoi(tokens[6])/255.0f);
				light.ambientIntensity = std::stof(tokens[7]);
				light.diffuseIntensity = std::stof(tokens[8]);
				light.specularIntensity = std::stof(tokens[9]);
			}else if (tokens[1] == "point_light") {
				id = std::strtoul(tokens[0].c_str(), &idEndPtr, 0);
				glm::vec3 position = glm::vec3(std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]));
				glm::vec3 color = glm::vec3(std::stoi(tokens[5])/255.0f, std::stoi(tokens[6])/255.0f, std::stoi(tokens[7])/255.0f);
				float diffuseIntensity = std::stof(tokens[8]);
				float specularIntensity = std::stof(tokens[9]);
				float constant = std::stof(tokens[10]);
				float linear = std::stof(tokens[11]);
				float quadratic = std::stof(tokens[12]);
				PointLight pointLight(position, constant, linear, quadratic);
				pointLight.id = id;
				pointLight.color = color;
				pointLight.diffuseIntensity = diffuseIntensity;
				pointLight.specularIntensity = specularIntensity;
				pointLights.push_back(pointLight);
			}else if (tokens[1] == "spot_light") {
				id = std::strtoul(tokens[0].c_str(), &idEndPtr, 0);
				glm::vec3 position = glm::vec3(std::stof(tokens[2]), std::stof(tokens[3]), std::stof(tokens[4]));
				glm::vec3 color = glm::vec3(std::stoi(tokens[5])/255.0f, std::stoi(tokens[6])/255.0f, std::stoi(tokens[7])/255.0f);
				float diffuseIntensity = std::stof(tokens[8]);
				float specularIntensity = std::stof(tokens[9]);
				float constant = std::stof(tokens[10]);
				float linear = std::stof(tokens[11]);
				float quadratic = std::stof(tokens[12]);
				glm::vec3 direction = glm::vec3(std::stof(tokens[13]), std::stof(tokens[14]), std::stof(tokens[15]));
				float cutoff = std::stof(tokens[16]);
				float outerCutoff = std::stof(tokens[17]);
				SpotLight spotLight(position, direction, constant, linear, quadratic, cutoff, outerCutoff);
				spotLight.id = id;
				spotLight.color = color;
				spotLight.diffuseIntensity = diffuseIntensity;
				spotLight.specularIntensity = specularIntensity;
				spotLights.push_back(spotLight);
			}
		}
	}
	// if no point lights,
	if (pointLights.size() == 0) {
		// add one unlit point light if none exists, so that the lighting shader compiles, as it expects at least one point light
		PointLight pointLight(glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);
		pointLights.push_back(pointLight);
	}
	// edit the lighting shader to reflect the number of point lights
	Utility::replaceLineInFile("shaders/lighting.glfs", 7, "#define NR_POINT_LIGHTS " + std::to_string(pointLights.size()));

}