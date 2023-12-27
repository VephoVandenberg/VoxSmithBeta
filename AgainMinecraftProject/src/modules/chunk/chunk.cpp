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

#include "block.h"
#include "chunk.h"

using namespace Engine::Renderer;

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr uint32_t g_nBlocks = g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;

constexpr uint32_t g_heightOffset = 100;

constexpr float g_rayDeltaMag = 0.1f;

struct BlockVert
{
	glm::ivec3 pos;
	uint8_t coordInd;
};

using VertexArray = std::array<BlockVert, g_vertexPerFace>;

constexpr VertexArray back{ {
	{{ 0, 0, 0 }, 0 },
	{{ 0, 1, 0 }, 2 },
	{{ 1, 0, 0 }, 1 },

	{{ 1, 0, 0 }, 1 },
	{{ 0, 1, 0 }, 2 },
	{{ 1, 1, 0 }, 3 },
} };

constexpr VertexArray front{ {
	{{ 0, 0, 1 }, 0 },
	{{ 1, 0, 1 }, 1 },
	{{ 0, 1, 1 }, 2 },

	{{ 1, 0, 1 }, 1 },
	{{ 1, 1, 1 }, 3 },
	{{ 0, 1, 1 }, 2 },
} };

constexpr VertexArray top{ {
	{{ 0, 1, 1 }, 0 },
	{{ 1, 1, 1 }, 1 },
	{{ 0, 1, 0 }, 2 },

	{{ 1, 1, 1 }, 1 },
	{{ 1, 1, 0 }, 3 },
	{{ 0, 1, 0 }, 2 },
} };

constexpr VertexArray bottom{ {
	{{ 0, 0, 1 }, 0 },
	{{ 0, 0, 0 }, 2 },
	{{ 1, 0, 1 }, 1 },

	{{ 1, 0, 0 }, 3 },
	{{ 1, 0, 1 }, 1 },
	{{ 0, 0, 0 }, 2 },
} };

constexpr VertexArray left{ {
	{{ 0, 0, 0 }, 0 },
	{{ 0, 0, 1 }, 1 },
	{{ 0, 1, 0 }, 2 },
				  
	{{ 0, 0, 1 }, 1 },
	{{ 0, 1, 1 }, 3 },
	{{ 0, 1, 0 }, 2 },
} };

constexpr VertexArray right{ {
	{{ 1, 0, 1 }, 0 },
	{{ 1, 0, 0 }, 1 },
	{{ 1, 1, 1 }, 2 },

	{{ 1, 0, 0 }, 1 },
	{{ 1, 1, 0 }, 3 },
	{{ 1, 1, 1 }, 2 }
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

BlockType getBlockType(Chunk& chunk, const glm::vec3& pos, float height)
{
	float stoneHeight = height - 5;
	float dirtHeight = height - 1;

	if (pos.y < stoneHeight)
	{
		return BlockType::STONE;
	}

	if (pos.y < dirtHeight)
	{
		return BlockType::DIRT;
	}

	if (pos.y == dirtHeight)
	{
		return BlockType::GRASS;
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

	std::array<uint32_t, g_chunkSize.x * g_chunkSize.z> heightMap;
	FastNoiseLite generator1;
	generator1.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	generator1.SetFractalType(FastNoiseLite::FractalType_FBm);
	generator1.SetFractalOctaves(6);
	generator1.SetFrequency(0.004f);
	generator1.SetFractalLacunarity(1.1f);
	generator1.SetFractalWeightedStrength(1.0f);

	FastNoiseLite generator2;
	generator2.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	generator2.SetFractalType(FastNoiseLite::FractalType_Ridged);
	generator2.SetFractalOctaves(5);
	generator2.SetFrequency(0.0014f);
	generator2.SetFractalLacunarity(0.9f);
	generator2.SetFractalWeightedStrength(0.7f);

	FastNoiseLite generator3;
	generator3.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	generator3.SetFractalType(FastNoiseLite::FractalType_FBm);
	generator3.SetFractalOctaves(4);
	generator3.SetFrequency(0.015f);
	generator3.SetFractalLacunarity(1.3f);
	generator3.SetFractalWeightedStrength(1.1f);

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
			blendedNoise = glm::pow(blendedNoise, 3.0f);
				

			heightMap[g_chunkSize.x * z + x] =
				//(g_heightOffset + 40.0f * noise1 + 40.0f * noise2 + 40.0f * noise3);
				(g_heightOffset + 100.0f * blendedNoise);
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

uint8_t getFaceId(BlockType type, Face::FaceType face)
{
	if (type == BlockType::GRASS)
	{
		switch (face)
		{
		case Face::FaceType::TOP:
			return 0;
		case Face::FaceType::BOTTOM:
			return 2;
		case Face::FaceType::RIGHT:
		case Face::FaceType::LEFT:
		case Face::FaceType::FRONT:
		case Face::FaceType::BACK:
			return 1;
		}
	}

	if (type == BlockType::STONE)
	{
		return 3;
	}
	
	if (type == BlockType::DIRT)
	{
		return 2;
	}
}

void updateFace(Chunk& chunk, const glm::ivec3 pos, BlockType type, Face::FaceType face)
{
	uint8_t texID = getFaceId(type, face);

	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		glm::ivec3 posData = pos + g_faces[face][iVertex].pos;

		int32_t data = 0;
		data |= (posData.x) & 0x1F;		  // x
		data |= (posData.y & 0x1FF) << 5; // y
		data |= (posData.z & 0x1F) << 14; // z

		data |= (g_faces[face][iVertex].coordInd & 0x3) << 19;	// Coord ind
		data |= (texID & 0xF) << 21;							// tex id

		chunk.mesh.vertices.push_back({data});
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

void GameModule::generateMesh(Chunk& chunk)
{

}

void GameModule::loadChunkMesh(Chunk& chunk)
{
	loadData(&chunk.mesh);
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

				if (less.blocks[iLess].type == BlockType::AIR &&
					more.blocks[iMore].type != BlockType::AIR)
				{
					setBlockFace(
						more,
						glm::vec3(0, y, z),
						more.blocks[iMore].type,
						Face::FaceType::LEFT);
				}
				else if (less.blocks[iLess].type != BlockType::AIR &&
					more.blocks[iMore].type == BlockType::AIR)
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

				if (less.blocks[iLess].type == BlockType::AIR &&
					more.blocks[iMore].type != BlockType::AIR)
				{
					setBlockFace(
						more, 
						glm::vec3(x, y, 0), 
						more.blocks[iMore].type,
						Face::FaceType::BACK);
				}
				else if (less.blocks[iLess].type != BlockType::AIR &&
					more.blocks[iMore].type == BlockType::AIR)
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

void GameModule::drawChunk(const Chunk& chunk)
{
	if (chunk.mesh.vertices.size() == 0)
	{
		return;
	}

	renderMesh(&chunk.mesh);
}
