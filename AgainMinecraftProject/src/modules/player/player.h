#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Camera;
}

namespace GameModule
{
	struct Player 
	{
		glm::vec3 pos;
		glm::vec3 velocity;
		uint32_t height;

		Engine::Camera camera;
	};
}