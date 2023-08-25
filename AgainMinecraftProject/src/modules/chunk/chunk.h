#pragma once

#include <glm/glm.hpp>
#include <stdint.h>

#include "../../engine/renderer/mesh.h"

namespace Engine
{
	struct Ray;
}

namespace GameModule
{
	struct Block;

	struct Chunk
	{
		glm::vec3 pos;
		Block* blocks;
		Engine::Renderer::Mesh mesh;
	};

	Chunk generateChunk(glm::vec3 pos);
	void loadChunkMesh(Chunk& chunk);
	void processRayInChunk(Chunk& chunk);
	bool rayInChunk(const Chunk& chunk, const Engine::Ray& ray);
	void checkNeighbourChunk(Chunk& chunk1, Chunk& chunk2);
	void generateMesh(Chunk& chunk);
	void drawChunk(const Chunk& chunk);
}
 