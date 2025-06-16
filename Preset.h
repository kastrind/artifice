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
					std::string tokens[11];
					while(std::getline(ss, token, ','))
					{
						if ( (i == 11 && tokens[0] == "directional") || (i == 10 && tokens[0] == "point")) break;
						tokens[++i] = token;
					}

					if (tokens[0] == "directional")
					{
						Light light = Light();
						directionalLights.push_back(light);
						//TODO: set light properties from tokens
					}
					else if (tokens[0] == "point")
					{
						PointLight pointLight = PointLight();
						pointLights.push_back(pointLight);
						//TODO: set point light properties from tokens
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
