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
		std::vector<Face>		faces = {};
		Engine::Renderer::Mesh	mesh;
	};

	enum class RayType
	{
		REMOVE,
		PLACE
	};

	Chunk	generateChunk(const glm::ivec3 pos);
	void	initMeshData(Chunk& chunk);
	void	generateMesh(Chunk& chunk);
	void	loadChunkMesh(Chunk& chunk);

	void	setBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);
	void	removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);

	void	drawChunk(const Chunk& chunk);
}
 