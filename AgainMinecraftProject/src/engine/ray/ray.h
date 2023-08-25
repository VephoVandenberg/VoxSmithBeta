#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Ray
	{
		glm::vec3 start;
		glm::vec3 end;
	};

	inline uint32_t getLength(const Ray& ray) { glm::length(ray.end - ray.start); }
}
