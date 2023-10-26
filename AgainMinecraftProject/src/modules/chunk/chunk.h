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
	
	// void	addBlock(Chunk& chunk, const glm::ivec3 iPos);
	// void	removeBlock(Chunk& chunk, const glm::ivec3 iPos);
	void	setBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);
	void	removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type);

	void	updateChunkNeighbourFace(Chunk& chunk1, Chunk& chunk2);
	//bool	processRayInChunk(Chunk& chunk, Engine::Ray& ray, RayType type = RayType::REMOVE);

	bool	rayStartInChunk(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInChunk(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderX(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderZ(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderXPos(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderZPos(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderXNeg(const Chunk& chunk, const Engine::Ray& ray);
	bool	rayEndInBorderZNeg(const Chunk& chunk, const Engine::Ray& ray);

	void	drawChunk(const Chunk& chunk);
}
 