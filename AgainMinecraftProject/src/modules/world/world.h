#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

namespace Engine
{
	struct Ray;
}

namespace GameModule
{
	enum class RayType;

	struct Chunk;
	struct Block;

	struct World
	{
		glm::ivec3 minBorder;
		glm::ivec3 maxBorder;
		
		struct KeyFuncs
		{
			size_t operator()(const glm::ivec3& v)const
			{
				return std::hash<int>()(v.x) ^ std::hash<int>()(v.y) ^ std::hash<int>()(v.z);
			}

			bool operator()(const glm::ivec3& a, const glm::vec3& b)const
			{
				return a.x == b.x && a.z == b.z;
			}
		};

		using ChunkMap = std::unordered_map<glm::ivec3, Chunk, KeyFuncs>;
		ChunkMap chunks;
	};

	void initWorld(World& world);
	void drawWorld(World& world);

	void processRay(World& world, Engine::Ray& ray, RayType type);
	void traceRay(World& world, glm::vec3 rayPosFrac, GameModule::RayType type);
}
