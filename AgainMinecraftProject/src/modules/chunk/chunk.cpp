#include <algorithm>
#include <glm/glm.hpp>

#include "../../engine/renderer/mesh.h"

#include "block.h"
#include "chunk.h"

using namespace GameModule;

constexpr uint32_t g_chunkSizeX = 16;
constexpr uint32_t g_chunkSizeY = 16;
constexpr uint32_t g_chunkSizeZ = 16;

constexpr uint32_t g_vertexPerCube = 36;
constexpr uint32_t g_vertexPerFace = 6;

Chunk GameModule::generateChunk(glm::vec3 pos)
{
	Chunk chunk;
	chunk.pos = pos;
	chunk.blocks = new Block[g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ];

	for (uint32_t x = 0; x < g_chunkSizeX; x++)
	{
		for (uint32_t y = 0; y < g_chunkSizeY; y++)
		{
			for (uint32_t z = 0; z < g_chunkSizeZ; z++)
			{
				auto index = y * g_chunkSizeX * g_chunkSizeZ + z * g_chunkSizeX + x;
				chunk.blocks[index].type = y < 8 ? BlockType::GRASS_DIRT : BlockType::AIR;
				chunk.blocks[index].pos = pos + glm::vec3(x, y, z);
			}
		}
	}

	return chunk;
}

void addBlcokkPosOffset(Engine::Renderer::Vertex* vertices, const glm::vec3& chunkPos, const size_t size)
{
	for (uint32_t vIndex = 0; vIndex < size; vIndex++)
	{
		vertices[vIndex].pos += chunkPos;
	}
}

bool checkAir(Block* blocks, uint32_t size, uint32_t index)
{
	if (index < size)
	{
		if (blocks[index].type == BlockType::AIR)
		{
			return true;
		}
	}
	return false;
}

void GameModule::generateMesh(Chunk* chunk)
{
	size_t nVertices = 
		g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ * 36;
	chunk->mesh.vertices = new Engine::Renderer::Vertex[nVertices];

	auto nBlocks = g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ;

	uint32_t iFace = 0;
	for (uint32_t iBlock = 0; iBlock < nBlocks; iBlock++)
	{
		if (chunk->blocks[iBlock].type == BlockType::AIR)
		{
			continue;
		}

		auto& pos = chunk->blocks[iBlock].pos;
		Engine::Renderer::Vertex back[] = {
			{pos + glm::vec3(0, 0, 0), { 0, 0 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},
			{pos + glm::vec3(1, 0, 0), { 1, 0 }},

			{pos + glm::vec3(1, 0, 0), { 1, 0 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},
			{pos + glm::vec3(1, 1, 0), { 1, 1 }},
		};
		Engine::Renderer::Vertex front[] = {
			{pos + glm::vec3(0, 0, 1), { 0, 0 }},
			{pos + glm::vec3(1, 0, 1), { 1, 0 }},
			{pos + glm::vec3(0, 1, 1), { 0, 1 }},

			{pos + glm::vec3(1, 0, 1), { 1, 0 }},
			{pos + glm::vec3(1, 1, 1), { 1, 1 }},
			{pos + glm::vec3(0, 1, 1), { 0, 1 }},
		};
		Engine::Renderer::Vertex top[] = {
			{pos + glm::vec3(0, 1, 1), { 0, 0 }},
			{pos + glm::vec3(1, 1, 1), { 1, 0 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},

			{pos + glm::vec3(1, 1, 1), { 1, 0 }},
			{pos + glm::vec3(1, 1, 0), { 1, 1 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},
		};
		Engine::Renderer::Vertex bottom[] = {
			{pos + glm::vec3(0, 0, 1), { 0, 0 }},
			{pos + glm::vec3(0, 0, 0), { 0, 1 }},
			{pos + glm::vec3(1, 0, 1), { 1, 0 }},

			{pos + glm::vec3(1, 0, 0), { 1, 1 }},
			{pos + glm::vec3(1, 0, 1), { 1, 0 }},
			{pos + glm::vec3(0, 0, 0), { 0, 1 }},
		};
		Engine::Renderer::Vertex left[] = {
			{pos + glm::vec3(0, 0, 0), { 0, 0 }},
			{pos + glm::vec3(0, 0, 1), { 1, 0 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},

			{pos + glm::vec3(0, 0, 1), { 1, 0 }},
			{pos + glm::vec3(0, 1, 1), { 1, 1 }},
			{pos + glm::vec3(0, 1, 0), { 0, 1 }},
		};
		Engine::Renderer::Vertex right[] = {
			{pos + glm::vec3(1, 0, 1), { 0, 0 }},
			{pos + glm::vec3(1, 0, 0), { 1, 0 }},
			{pos + glm::vec3(1, 1, 1), { 0, 1 }},

			{pos + glm::vec3(1, 0, 0), { 1, 0 }},
			{pos + glm::vec3(1, 1, 0), { 1, 1 }},
			{pos + glm::vec3(1, 1, 1), { 0, 1 }}
		};

		auto iPos = pos - chunk->pos;
		uint32_t iTop = (iPos.y + 1) * g_chunkSizeX * g_chunkSizeZ + iPos.z * g_chunkSizeX + iPos.x;
		uint32_t iFront = iPos.y * g_chunkSizeX * g_chunkSizeZ + (iPos.z + 1) * g_chunkSizeX + iPos.x;
		uint32_t iRight = iPos.y * g_chunkSizeX * g_chunkSizeZ + iPos.z * g_chunkSizeX + (iPos.x + 1);

		// Remake that shit
		if (checkAir(chunk->blocks, nBlocks, iTop))
		{
			
		}
		else
		{
			
		}

		if (checkAir(chunk->blocks, nBlocks, iFront))
		{
			
		}
		else
		{
			
		}

		if (checkAir(chunk->blocks, nBlocks, iRight))
		{
			
		}
		else
		{
			
		}
	}
	chunk->mesh.size = sizeof(Engine::Renderer::Vertex) * nBlocks;
	Engine::Renderer::loadData(&chunk->mesh);
}

void GameModule::drawChunk(const Chunk* chunk)
{
	if (!chunk)
	{
		return;
	}

	Engine::Renderer::renderMesh(&chunk->mesh);
}
