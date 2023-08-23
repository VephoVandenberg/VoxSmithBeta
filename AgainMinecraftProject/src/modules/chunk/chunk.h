#pragma once

#include <glm/glm.hpp>
#include <stdint.h>

#include "../../engine/renderer/mesh.h"

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
	void generateMesh(Chunk* chunk);
	void drawChunk(const Chunk* chunk);
}
 