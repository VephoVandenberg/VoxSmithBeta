#pragma once

#include <glm/glm.hpp>
#include <stdint.h>
#include <queue>
#include <vector>

#include "../../engine/renderer/mesh.h"

#include "block.h"

namespace Engine
{
	struct Ray;
}

namespace GameModule
{
	struct Block;

	struct Chunk
	{
		Chunk() = default;
		~Chunk() { deleteMesh(&solidMesh); }

		Chunk*					front		= nullptr;
		Chunk*					back		= nullptr;
		Chunk*					left		= nullptr;
		Chunk*					right		= nullptr;

		bool					updated		= false;
		glm::vec3				pos;
		std::vector<Block>		blocks		= {};
		Engine::Renderer::Mesh	solidMesh;
		Engine::Renderer::Mesh	transparentMesh;
	};

	enum class ChunkDir : int8_t
	{
		NORTH,
		SOUTH,
		EAST,
		WEST
	};

	enum class RayType
	{
		IDLE,
		REMOVE,
		PLACE
	};

	Chunk	generateChunk(const glm::ivec3& pos);
	void	loadChunkMesh(Chunk& chunk);
	void	updateChunkNeighbourFace(Chunk& chunk1, Chunk& chunk2);

	void	setType(Block& block);
	void	setBlockFace(Chunk& chunk, const glm::vec3& pos, BlockType type, Face::FaceType face);
	void	removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);

	void	drawSolid(const Chunk& chunk);
	void	drawTrans(const Chunk& chunk);
}
 