
#pragma once

#include <glm/glm.hpp>

typedef struct Light {

	public:

		float intensity = 1.0f;

		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

} Light;