#pragma once

#include <glm/glm.hpp>

namespace GameModule
{
	enum class BlockType
	{
		AIR,
		GRASS_DIRT,
		DIRT,
		STONE,
		SAND,
		SNOW,
	};

	struct Block
	{
		BlockType type;
		glm::vec3 pos;
	};
}
