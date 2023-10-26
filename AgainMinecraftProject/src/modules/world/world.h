#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

namespace Engine
{
	struct Ray;
}

namespace GameModule
{
	struct Chunk;
	struct Block;

	struct World
	{
		std::unordered_map<glm::ivec3, Chunk> chunks;
	};

	void update(World& world);
	void draw(const World& world);
}