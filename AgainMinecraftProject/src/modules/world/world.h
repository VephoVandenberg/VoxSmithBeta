#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

namespace Engine
{
	struct Ray;
	struct Shader;
}

namespace GameModule
{
	enum class RayType;

	struct Chunk;
	struct Block;
	struct Player;

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
	void initChunkFaces(World& world, Chunk& chunk);
	void updateWorld(World& world, Player& player);
	void drawWorld(World& world);

	void processRay(World& world, const Player& player, Engine::Ray& ray, Engine::Shader& shader, RayType type);
	void traceRay(World& world, glm::vec3 rayPosFrac, Engine::Shader& shader, RayType type);
	
	void checkPlayerCollision(World& world, Player& player, float dt);

	bool isPlayerFalling(World& world, const Player& player, float dt);
}
