/**
* Steps to draw the chunks and
* check and update neighbour chunks if needed.
*
* 1. Generate blocks.
* 2. Init mesh data
* 3. Load mesh data.
*/

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <array>
#include <FastNoiseLite.h>

#include "../../engine/renderer/mesh.h"
#include "../../engine/ray/ray.h"
#include "../../engine/texture/texture_list.h"

#include "block.h"
#include "chunk.h"

using namespace Engine::Renderer;

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr uint32_t g_nBlocks = g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;

constexpr uint32_t g_heightOffset = 80;

constexpr float g_rayDeltaMag = 0.1f;

struct BlockVert
{
	glm::ivec3 pos;
	uint8_t coordInd;
	float ambient;
};

using VertexArray = std::array<BlockVert, g_vertexPerFace>;

constexpr VertexArray back{ {
	{{ 0, 0, 0 }, 0, 0.7f },
	{{ 0, 1, 0 }, 2, 0.7f },
	{{ 1, 0, 0 }, 1, 0.7f },

	{{ 1, 0, 0 }, 1, 0.7f },
	{{ 0, 1, 0 }, 2, 0.7f },
	{{ 1, 1, 0 }, 3, 0.7f },
} };

constexpr VertexArray front{ {
	{{ 0, 0, 1 }, 0, 0.7f },
	{{ 1, 0, 1 }, 1, 0.7f },
	{{ 0, 1, 1 }, 2, 0.7f },

	{{ 1, 0, 1 }, 1, 0.7f },
	{{ 1, 1, 1 }, 3, 0.7f },
	{{ 0, 1, 1 }, 2, 0.7f },
} };

constexpr VertexArray top{ {
	{{ 0, 1, 1 }, 0, 1.0f },
	{{ 1, 1, 1 }, 1, 1.0f },
	{{ 0, 1, 0 }, 2, 1.0f },

	{{ 1, 1, 1 }, 1, 1.0f },
	{{ 1, 1, 0 }, 3, 1.0f },
	{{ 0, 1, 0 }, 2, 1.0f },
} };

constexpr VertexArray bottom{ {
	{{ 0, 0, 1 }, 0, 0.2f },
	{{ 0, 0, 0 }, 2, 0.2f },
	{{ 1, 0, 1 }, 1, 0.2f },

	{{ 1, 0, 0 }, 3, 0.2f },
	{{ 1, 0, 1 }, 1, 0.2f },
	{{ 0, 0, 0 }, 2, 0.2f },
} };

constexpr VertexArray left{ {
	{{ 0, 0, 0 }, 0, 0.4f },
	{{ 0, 0, 1 }, 1, 0.4f },
	{{ 0, 1, 0 }, 2, 0.4f },

	{{ 0, 0, 1 }, 1, 0.4f },
	{{ 0, 1, 1 }, 3, 0.4f },
	{{ 0, 1, 0 }, 2, 0.4f },
} };

constexpr VertexArray right{ {
	{{ 1, 0, 1 }, 0, 0.9f },
	{{ 1, 0, 0 }, 1, 0.9f },
	{{ 1, 1, 1 }, 2, 0.9f },

	{{ 1, 0, 0 }, 1, 0.9f },
	{{ 1, 1, 0 }, 3, 0.9f },
	{{ 1, 1, 1 }, 2, 0.9f }
} };

using FaceMap = std::unordered_map<Face::FaceType, const VertexArray, EnumHash>;
FaceMap g_faces = {
	{Face::FaceType::TOP,		top},
	{Face::FaceType::BOTTOM,	bottom},
	{Face::FaceType::FRONT,		front},
	{Face::FaceType::BACK,		back},
	{Face::FaceType::RIGHT,		right},
	{Face::FaceType::LEFT,		left},
};

void GameModule::setType(Block& block)
{
}

BlockType getBlockType(Chunk& chunk, const glm::vec3& pos, const float height)
{
	float waterLevel = 105.0f;

	if (pos.y <= height)
	{
		float dirtHeight = height - 3;
		float mountainLevel = 135.0f;
		float peakLevel = 150.0f;

		if (pos.y > peakLevel)
		{
			return BlockType::SNOW;
		}
		if (height > mountainLevel)
		{
			return BlockType::STONE;
		}

		if (pos.y == height)
		{
			if (pos.y > waterLevel + 1)
			{
				return BlockType::GRASS;
			}
			return BlockType::SAND;
		}
		if (pos.y > dirtHeight)
		{
			return BlockType::DIRT;
		}

		return BlockType::STONE;
	}

	if (pos.y < waterLevel)
	{
		return BlockType::WATER;
	}

	return BlockType::AIR;
}

Chunk GameModule::generateChunk(const glm::ivec3& pos)
{
	Chunk chunk;
	chunk.pos = pos;
	if (chunk.blocks.empty())
	{
		chunk.blocks.reserve(g_nBlocks);
	}

	std::array<uint32_t, g_chunkSize.x* g_chunkSize.z> heightMap;
	FastNoiseLite generator1;
	generator1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	generator1.SetFractalType(FastNoiseLite::FractalType_FBm);
	generator1.SetFractalOctaves(6);
	generator1.SetFrequency(0.0024f);
	generator1.SetFractalLacunarity(1.4f);
	generator1.SetFractalWeightedStrength(1.0f);

	FastNoiseLite generator2;
	generator2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	generator2.SetFractalType(FastNoiseLite::FractalType_Ridged);
	generator2.SetFractalOctaves(3);
	generator2.SetFrequency(0.00147f);
	generator2.SetFractalLacunarity(1.0f);
	generator2.SetFractalWeightedStrength(0.3f);

	FastNoiseLite generator3;
	generator3.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	generator3.SetFractalType(FastNoiseLite::FractalType_FBm);
	generator3.SetFractalOctaves(5);
	generator3.SetFrequency(0.015f);
	generator3.SetFractalLacunarity(1.3f);
	generator3.SetFractalWeightedStrength(0.7f);

	for (int32_t z = 0; z < g_chunkSize.z; z++)
	{
		for (int32_t x = 0; x < g_chunkSize.x; x++)
		{
			float noise1 = generator1.GetNoise(
				static_cast<float>(pos.x + x),
				static_cast<float>(pos.z + z));

			float noise2 = generator2.GetNoise(
				static_cast<float>(pos.x + x),
				static_cast<float>(pos.z + z));

			float noise3 = generator3.GetNoise(
				static_cast<float>(pos.x + x),
				static_cast<float>(pos.z + z));

			float blendedNoise = (noise1 + noise2 + noise3) / 3.343f + 0.5f;
			blendedNoise = glm::pow(blendedNoise, 2.477f);


			heightMap[g_chunkSize.x * z + x] =
				(g_heightOffset + 130.0f * blendedNoise);
		}
	}

	for (int32_t y = 0; y < g_chunkSize.y; y++)
	{
		for (int32_t z = 0; z < g_chunkSize.z; z++)
		{
			for (int32_t x = 0; x < g_chunkSize.x; x++)
			{
				Block block;
				uint32_t heightId = g_chunkSize.x * z + x;
				block.type = getBlockType(chunk, chunk.pos + glm::vec3(x, y, z), heightMap[heightId]);
				chunk.blocks.push_back(block);
			}
		}
	}

	return chunk;
}

Engine::TextureId getFaceId(BlockType type, Face::FaceType face)
{
	if (type == BlockType::GRASS)
	{
		switch (face)
		{
		case Face::FaceType::TOP:
			return Engine::TextureId::DIRT_GRASS;

		case Face::FaceType::BOTTOM:
			return Engine::TextureId::DIRT;

		case Face::FaceType::RIGHT:
		case Face::FaceType::LEFT:
		case Face::FaceType::FRONT:
		case Face::FaceType::BACK:
			return Engine::TextureId::DIRT_GRASS;
		}
	}

	if (type == BlockType::STONE)
	{
		return Engine::TextureId::STONE;
	}

	if (type == BlockType::DIRT)
	{
		return Engine::TextureId::DIRT;
	}

	if (type == BlockType::SNOW)
	{
		return Engine::TextureId::SNOW;
	}

	if (type == BlockType::SAND)
	{
		return Engine::TextureId::SAND;
	}

	if (type == BlockType::WATER)
	{
		return Engine::TextureId::WATER;
	}
}

void updateFace(Chunk& chunk, const glm::ivec3 pos, BlockType type, Face::FaceType face)
{
	uint8_t texID = static_cast<const uint8_t>(getFaceId(type, face));

	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		glm::ivec3 posData = pos + g_faces[face][iVertex].pos;
		float ambient = g_faces[face][iVertex].ambient;

		int32_t data = 0;
		data |= (posData.x) & 0x1F;		  // x
		data |= (posData.y & 0x1FF) << 5; // y
		data |= (posData.z & 0x1F) << 14; // z

		data |= (g_faces[face][iVertex].coordInd & 0x3) << 19;	// Coord ind
		data |= (texID & 0xF) << 21;							// tex id

		data |= (static_cast<int32_t>(ambient * 10) & 0xF) << 25;

		if (type == BlockType::WATER)
		{
			chunk.transparentMesh.vertices.push_back({ data });
		}
		else
		{
			chunk.solidMesh.vertices.push_back({ data });
		}
	}
}

void GameModule::setBlockFace(Chunk& chunk, const glm::vec3& pos, BlockType type, Face::FaceType face)
{
	chunk.updated = false;
	updateFace(chunk, pos, type, face);
}

void GameModule::removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	chunk.updated = false;
}

void GameModule::loadChunkMesh(Chunk& chunk)
{
	loadData(&chunk.solidMesh);
	loadData(&chunk.transparentMesh);
}

void GameModule::updateChunkNeighbourFace(Chunk& chunk1, Chunk& chunk2)
{
	auto& less =
		chunk1.pos.x < chunk2.pos.x || chunk1.pos.z < chunk2.pos.z ? chunk1 : chunk2;
	auto& more =
		chunk1.pos.x > chunk2.pos.x || chunk1.pos.z > chunk2.pos.z ? chunk1 : chunk2;
	if (less.pos.x < more.pos.x)
	{
		for (uint32_t y = 0; y < g_chunkSize.y; y++)
		{
			for (uint32_t z = 0; z < g_chunkSize.z; z++)
			{
				uint32_t iLess =
					g_chunkSize.x * (y * g_chunkSize.z + z + 1) - 1;
				uint32_t iMore =
					g_chunkSize.x * (y * g_chunkSize.z + z) + 0;

				bool moreSolid = more.blocks[iMore].type != BlockType::AIR && more.blocks[iMore].type != BlockType::WATER;
				bool lessSolid = less.blocks[iLess].type != BlockType::AIR && less.blocks[iLess].type != BlockType::WATER;

				if (less.blocks[iLess].type == BlockType::WATER && moreSolid ||
					less.blocks[iLess].type == BlockType::AIR && moreSolid ||
					less.blocks[iLess].type == BlockType::AIR && more.blocks[iMore].type == BlockType::WATER)
				{
					setBlockFace(
						more,
						glm::vec3(0, y, z),
						more.blocks[iMore].type,
						Face::FaceType::LEFT);
				}
				else if(more.blocks[iMore].type == BlockType::WATER && lessSolid ||
					more.blocks[iMore].type == BlockType::AIR && lessSolid ||
					more.blocks[iMore].type == BlockType::AIR && less.blocks[iLess].type == BlockType::WATER)
				{
					setBlockFace(
						less,
						glm::vec3(g_chunkSize.x - 1, y, z),
						less.blocks[iLess].type,
						Face::FaceType::RIGHT);
				}
			}
		}
	}
	else if (less.pos.z < more.pos.z)
	{
		for (uint32_t y = 0; y < g_chunkSize.y; y++)
		{
			for (uint32_t x = 0; x < g_chunkSize.x; x++)
			{
				uint32_t iLess =
					y * g_chunkSize.x * g_chunkSize.z + (g_chunkSize.z - 1) * g_chunkSize.x + x;
				uint32_t iMore =
					y * g_chunkSize.x * g_chunkSize.z + (0) * g_chunkSize.x + x;

				bool moreSolid = more.blocks[iMore].type != BlockType::AIR && more.blocks[iMore].type != BlockType::WATER;
				bool lessSolid = less.blocks[iLess].type != BlockType::AIR && less.blocks[iLess].type != BlockType::WATER;

				if (less.blocks[iLess].type == BlockType::WATER && moreSolid ||
					less.blocks[iLess].type == BlockType::AIR && moreSolid ||
					less.blocks[iLess].type == BlockType::AIR && more.blocks[iMore].type == BlockType::WATER)
				{
					setBlockFace(
						more,
						glm::vec3(x, y, 0),
						more.blocks[iMore].type,
						Face::FaceType::BACK);
				}
				else if (more.blocks[iMore].type == BlockType::WATER && lessSolid ||
					more.blocks[iMore].type == BlockType::AIR && lessSolid ||
					more.blocks[iMore].type == BlockType::AIR && less.blocks[iLess].type == BlockType::WATER)
				{
					setBlockFace(
						less,
						glm::vec3(x, y, g_chunkSize.z - 1),
						less.blocks[iLess].type,
						Face::FaceType::FRONT);
				}
			}
		}
	}
}

void GameModule::drawSolid(const Chunk& chunk)
{
	if (chunk.solidMesh.vertices.size() == 0)
	{
		return;
	}

	renderMesh(&chunk.solidMesh);
}

void GameModule::drawTrans(const Chunk& chunk)
{
	if (chunk.transparentMesh.vertices.size() == 0)
	{
		return;
	}

	renderMesh(&chunk.transparentMesh);
}

