#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Configuration.h"
#include "Light.h"

class Preset 
{
	public:

		void loadPresetLights() {
			
			std::string presetLightingPath = presetPath + cfg.PATH_SEP + "lighting.txt";
			printf( "Loading preset lights from %s\n",  presetLightingPath.c_str() );

			std::ifstream f(presetLightingPath);
			if (!f.is_open())
			{
				printf( "Failed to load preset lights!\n" );
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
				char* idEndPtr;

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
					std::string tokens[12];
					while(std::getline(ss, token, ','))
					{
						if ( (i == 11 && tokens[0] == "directional") ||
							 (i == 10 && tokens[0] == "point") ||
							 (i == 12 && tokens[0] == "spot") ) break;
						tokens[++i] = token;
					}

					if (tokens[0] == "directional")
					{
						Light light = Light();
						light.name = tokens[1];
						light.direction.x = std::stof(tokens[2]);
						light.direction.y = std::stof(tokens[3]);
						light.direction.z = std::stof(tokens[4]);
						light.color.r = std::stof(tokens[5]) / 255.0f;
						light.color.g = std::stof(tokens[6]) / 255.0f;
						light.color.b = std::stof(tokens[7]) / 255.0f;
						light.ambientIntensity = std::stof(tokens[8]);
						light.diffuseIntensity = std::stof(tokens[9]);
						light.specularIntensity = std::stof(tokens[10]);
						directionalLights.push_back(light);
					}
					else if (tokens[0] == "point")
					{
						PointLight pointLight = PointLight();
						pointLight.name = tokens[1];
						pointLight.color.r = std::stof(tokens[2]) / 255.0f;
						pointLight.color.g = std::stof(tokens[3]) / 255.0f;
						pointLight.color.b = std::stof(tokens[4]) / 255.0f;
						pointLight.diffuseIntensity = std::stof(tokens[5]);
						pointLight.specularIntensity = std::stof(tokens[6]);
						pointLight.constant = std::stof(tokens[7]);
						pointLight.linear = std::stof(tokens[8]);
						pointLight.quadratic = std::stof(tokens[9]);
						pointLights.push_back(pointLight);
					}
					else if (tokens[0] == "spot")
					{
						SpotLight spotLight = SpotLight();
						spotLight.name = tokens[1];
						spotLight.color.r = std::stof(tokens[2]) / 255.0f;
						spotLight.color.g = std::stof(tokens[3]) / 255.0f;
						spotLight.color.b = std::stof(tokens[4]) / 255.0f;
						spotLight.diffuseIntensity = std::stof(tokens[5]);
						spotLight.specularIntensity = std::stof(tokens[6]);
						spotLight.constant = std::stof(tokens[7]);
						spotLight.linear = std::stof(tokens[8]);
						spotLight.quadratic = std::stof(tokens[9]);
						spotLight.cutoff = std::stof(tokens[10]);
						spotLight.outerCutoff = std::stof(tokens[11]);
						spotLights.push_back(spotLight);
					}
				}
			}
		}

		std::vector<Light>& getDirectionalLights() {
			return directionalLights;
		}

		std::vector<PointLight>& getPointLights() {
			return pointLights;
		}

		std::vector<SpotLight>& getSpotLights() {
			return spotLights;
		}

	private:

		std::string presetPath = cfg.ASSETS_PATH + cfg.PATH_SEP + "presets";

		std::vector<Light> directionalLights;

		std::vector<PointLight> pointLights;

		std::vector<SpotLight> spotLights;

};
