
#pragma once

#include <glm/glm.hpp>

typedef struct AmbientLight {

	public:

		float intensity = 1.0f;

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

} AmbientLight;

typedef struct Light {

	public:

		glm::vec3 position;

		glm::vec3 diffuse;

		glm::vec3 specular;

} Light;