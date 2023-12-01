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
		bool updated = false;
		glm::vec3 pos;
		std::vector<Block>		blocks = {};
		Engine::Renderer::Mesh	mesh;
	};

	enum class RayType
	{
		IDLE,
		REMOVE,
		PLACE
	};

	Chunk	generateChunk(const glm::ivec3 pos);
	void	generateMesh(Chunk& chunk);
	void	loadChunkMesh(Chunk& chunk);

	void	setType(Block& block);
	void	setBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);
	void	removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);
	//void	updateFace(Engine::Renderer::Vertex* vertices, const Block& block, uint32_t texID);


	void	drawChunk(const Chunk& chunk);
}
 