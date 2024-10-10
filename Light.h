
#pragma once

#include <glm/glm.hpp>

class Light {

	public:

		glm::vec3 direction;

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

		float ambientIntensity = 0.2f;

		float diffuseIntensity = 0.5f;

		float specularIntensity = 0.9f;

};