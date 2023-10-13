#pragma once

#include <glm/glm.hpp>
#include <stdint.h>

#include "block.h"

namespace Engine
{
	struct Ray;

	namespace Renderer 
	{
		struct Mesh;
	}
}

namespace GameModule
{
	struct Chunk
	{
		glm::vec3 pos;
		Block* blocks = nullptr;
		Face* faces = nullptr;
		size_t nFaces;
		Engine::Renderer::Mesh mesh;
	};

	enum class RayType
	{
		REMOVE,
		PLACE
	};

	Chunk generateChunk(const glm::vec3 pos);
	void deleteChunk(Chunk& chunk);
	void initMeshData(Chunk& chunk);
	void generateMesh(Chunk& chunk);
	void loadChunkMesh(Chunk& chunk);

	void updateChunkNeighbourFace(Chunk& chunk1, Chunk& chunk2);
	bool processRayInChunk(Chunk& chunk, Engine::Ray& ray, RayType type = RayType::REMOVE);

	bool rayStartInChunk(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInChunk(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderX(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderZ(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderXPos(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderZPos(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderXNeg(const Chunk& chunk, const Engine::Ray& ray);
	bool rayEndInBorderZNeg(const Chunk& chunk, const Engine::Ray& ray);


	void drawChunk(const Chunk& chunk);
}
 