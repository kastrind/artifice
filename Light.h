
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

class PointLight : public Light {

	public:

		glm::vec3 position;

		float constant = 1.0f;

		float linear = 0.7f;

		float quadratic = 1.8f;

};

class SpotLight : public PointLight {

	public:

		float cutOff;

		float outerCutOff;

};