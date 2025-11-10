
#pragma once

#include <glm/glm.hpp>
#include <string>

class Light {

	public:

		unsigned long id = 0;

		glm::vec3 direction;

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

		float ambientIntensity = 0.2f;

		float diffuseIntensity = 0.5f;

		float specularIntensity = 0.9f;

		std::string name = "directional";

};

class PointLight : public Light {

	public:
		PointLight() { name = "point"; }

		PointLight(glm::vec3 position, float constant, float linear, float quadratic)
					: position(position), constant(constant), linear(linear), quadratic(quadratic) {
			cutoffDistance =  solveAttenuationCutoff(constant, linear, quadratic, 0.01f);
			if (cutoffDistance < 0.0f) {
				cutoffDistance = 10000.0f; // no cutoff, set to a large value
			}
			name = "point";
		}


		glm::vec3 position;

		float constant = 1.0f;

		float linear = 0.7f;

		float quadratic = 1.8f;

		float cutoffDistance = 7.0f;

	private:

		float solveAttenuationCutoff(float kc, float kl, float kq, float epsilon) {
			float c = kc - 1.0f / epsilon;
			float discriminant = kl * kl - 4.0f * kq * c;
			if (discriminant < 0.0f) return -1.0f; // no real solution
			float sqrtDisc = std::sqrt(discriminant);
			return (-kl + sqrtDisc) / (2.0f * kq); // positive root
		}

};

class SpotLight : public PointLight {

	public:
		SpotLight() { name = "spot"; }

		SpotLight(glm::vec3 position, glm::vec3 direction, float constant, float linear, float quadratic, float cutoff, float outerCutoff)
					: PointLight(position, constant, linear, quadratic), cutoff(cutoff), outerCutoff(outerCutoff) {
			name = "spot";
			this->direction = direction;
		}

		float cutoff = 15.0f;

		float outerCutoff = 20.0f;

};