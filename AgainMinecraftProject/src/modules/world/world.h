#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>

#include <glm/glm.hpp>

#include "../../engine/renderer/mesh.h"
#include "../../engine/texture/framebuffer.h"

namespace Engine
{
	struct Ray;
	struct Shader;
	struct FBuffer;

	namespace Renderer
	{
		template <uint32_t N>
		struct BufferPool;
	}

	void enableCulling();
	void disableCulling();
}

static constexpr int32_t g_chunksX = 24;
static constexpr int32_t g_chunksZ = 24;

namespace GameModule
{
	enum class RayType;

	struct Chunk;
	struct Block;
	struct Player;

	struct World
	{
		glm::ivec3 pos;
		glm::vec3 fractionPos;
		
		glm::vec3 lightDir = glm::normalize(glm::vec3(40.0f, 25.0f, 0.0f));

		float updateRadius;
		
		struct KeyFuncs
		{
			size_t operator()(const glm::ivec3& v) const
			{
				size_t h = 0xcf234123f;
				h ^= std::_Bit_cast<int32_t>(v.x) + 0x9e3779b9 + (h << 6) + (h >> 2);
				h ^= std::_Bit_cast<int32_t>(v.y) + 0x9e3779b9 + (h << 6) + (h >> 2);
				h ^= std::_Bit_cast<int32_t>(v.z) + 0x9e3779b9 + (h << 6) + (h >> 2);
				return h;
			}

			bool operator()(const glm::ivec3& a, const glm::ivec3& b) const
			{
				return a.x == b.x && a.z == b.z;
			}
		};

		std::unordered_map<glm::ivec3, Chunk, KeyFuncs> chunks;

		Engine::Renderer::BufferPool<2 * g_chunksX * g_chunksZ> pool; // We need for every chunk 2 meshes

		std::unordered_set<glm::ivec3, KeyFuncs> chunksToRemove;
		std::unordered_set<glm::ivec3, KeyFuncs> chunksToAdd;

		uint32_t threadsAvailable;

		std::vector<float> shadowCascadeLevels;

		Engine::FBuffer shadowBuffer;
		Engine::Renderer::UBuffer lightSpaceMatricesUBO;
	};

	void initWorld(World& world, const Player& player);
	void initChunkFaces(Chunk& chunk);
	void updateWorld(World& world, const Player& player, float dt);

	void drawWorld(World& world, const Player& player, Engine::Shader& shader);
	void drawWorlToSM(World& world, Player& player, Engine::Shader& shader);
#ifdef _DEBUG
	void drawDebugQuad(World& world, Engine::Shader& shader);
#endif

	void processRay(World& world, const Player& player, Engine::Ray& ray, Engine::Shader& shader, RayType type);
	void traceRay(World& world, glm::vec3 rayPosFrac, Engine::Shader& shader, RayType type);
	
	void checkPlayerCollision(World& world, Player& player, float dt);

	bool isPlayerFalling(World& world, const Player& player, float dt);
}
