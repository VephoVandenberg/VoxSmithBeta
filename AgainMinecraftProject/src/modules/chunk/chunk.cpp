/**
* Steps to draw the chunks and
* check and update neighbour chunks if needed.
*
* 1. Generate blocks.
* 2. Init mesh data,
*	 that means we are loading up faces
*	 to the array with the correspoding vertices, also we store the block ID(index),
*	 and type of face that will be drawn.
* 3. Sort the faces array by block id, then generate the mesh.
*/

#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>
#include <array>

#include "../../engine/renderer/mesh.h"
#include "../../engine/ray/ray.h"

#include "block.h"
#include "chunk.h"

using namespace Engine::Renderer;

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr uint32_t g_nBlocks		= g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;
constexpr uint32_t g_vertexPerCube	= 36;

constexpr float g_rayDeltaMag		= 0.1f;

using VertexArray = std::array<Vertex, g_vertexPerFace>;

constexpr VertexArray back{ {
	{{ 0, 0, 0 }, { 0, 0, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},
	{{ 1, 0, 0 }, { 1, 0, -1 }},

	{{ 1, 0, 0 }, { 1, 0, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},
	{{ 1, 1, 0 }, { 1, 1, -1 }},
} };

constexpr VertexArray front{ {
	{{ 0, 0, 1 }, { 0, 0, -1 }},
	{{ 1, 0, 1 }, { 1, 0, -1 }},
	{{ 0, 1, 1 }, { 0, 1, -1 }},

	{{ 1, 0, 1 }, { 1, 0, -1 }},
	{{ 1, 1, 1 }, { 1, 1, -1 }},
	{{ 0, 1, 1 }, { 0, 1, -1 }},
} };

constexpr VertexArray top{ {
	{{ 0, 1, 1 }, { 0, 0, -1 }},
	{{ 1, 1, 1 }, { 1, 0, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},

	{{ 1, 1, 1 }, { 1, 0, -1 }},
	{{ 1, 1, 0 }, { 1, 1, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},
} };

constexpr VertexArray bottom{ {
	{{ 0, 0, 1 }, { 0, 0, -1 }},
	{{ 0, 0, 0 }, { 0, 1, -1 }},
	{{ 1, 0, 1 }, { 1, 0, -1 }},

	{{ 1, 0, 0 }, { 1, 1, -1 }},
	{{ 1, 0, 1 }, { 1, 0, -1 }},
	{{ 0, 0, 0 }, { 0, 1, -1 }},
} };

constexpr VertexArray left{ {
	{{ 0, 0, 0 }, { 0, 0, -1 }},
	{{ 0, 0, 1 }, { 1, 0, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},

	{{ 0, 0, 1 }, { 1, 0, -1 }},
	{{ 0, 1, 1 }, { 1, 1, -1 }},
	{{ 0, 1, 0 }, { 0, 1, -1 }},
} };

constexpr VertexArray right{ {
	{{ 1, 0, 1 }, { 0, 0, -1 }},
	{{ 1, 0, 0 }, { 1, 0, -1 }},
	{{ 1, 1, 1 }, { 0, 1, -1 }},

	{{ 1, 0, 0 }, { 1, 0, -1 }},
	{{ 1, 1, 0 }, { 1, 1, -1 }},
	{{ 1, 1, 1 }, { 0, 1, -1 }}
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
	switch (block.type)
	{
	case BlockType::GRASS:
		block.texIDs[static_cast<uint32_t>(Face::FaceType::TOP)]		= 0;
		block.texIDs[static_cast<uint32_t>(Face::FaceType::RIGHT)]		= 1;
		block.texIDs[static_cast<uint32_t>(Face::FaceType::LEFT)]		= 1;
		block.texIDs[static_cast<uint32_t>(Face::FaceType::FRONT)]		= 1;
		block.texIDs[static_cast<uint32_t>(Face::FaceType::BACK)]		= 1;
		block.texIDs[static_cast<uint32_t>(Face::FaceType::BOTTOM)]		= 2;
		break;

	case BlockType::DIRT:
		for (int32_t i = 0; i < g_facePerCube; i++)
		{
			block.texIDs[i] = 2;
		}
		break;
	}
}

Chunk GameModule::generateChunk(const glm::ivec3 pos)
{
	Chunk chunk;
	chunk.pos = pos;
	if (chunk.blocks.empty())
	{
		chunk.blocks.reserve(g_nBlocks);
	}

	for (int32_t y = 0; y < g_chunkSize.y; y++)
	{
		for (int32_t z = 0; z < g_chunkSize.z; z++)
		{
			for (int32_t x = 0; x < g_chunkSize.x; x++)
			{
				Block block;
				block.pos = chunk.pos + glm::vec3(x, y, z);

				if (y < g_chunkSize.y / 2 - 1)
				{
					block.type = BlockType::DIRT;
				}
				else if (y == g_chunkSize.y / 2 - 1)
				{
					block.type = BlockType::GRASS;
				}
				else
				{
					block.type = BlockType::AIR;
				}

				setType(block);
				
				chunk.blocks.push_back(block);
			}
		}
	}

	return chunk;
}

void updateFace(Vertex* vertices, const Block& block, uint32_t texID)
{
	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		vertices[iVertex].pos += block.pos;
		vertices[iVertex].uvw.z = texID;
	}
}

void GameModule::setBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	chunk.updated = false;
	Vertex vertices[g_vertexPerFace];
	std::copy(g_faces[type].begin(), g_faces[type].end(), vertices);
	updateFace(vertices, chunk.blocks[id], chunk.blocks[id].texIDs[static_cast<uint32_t>(type)]);

	Face face_;
	face_.type = type;
	face_.blockID = id;
	std::copy(vertices, vertices + g_vertexPerFace, face_.vertices);

	chunk.faces.push_back(face_);
}

void GameModule::removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	chunk.updated = false;
	chunk.faces.erase(
		std::remove_if(
			chunk.faces.begin(), chunk.faces.end(),
			[&](const Face& face) {
				return face.blockID == id && face.type == type;
			}),
		chunk.faces.end()
	);
}

void GameModule::initMeshData(Chunk& chunk)
{
	if (chunk.faces.empty())
	{
		chunk.faces.reserve(g_nBlocks * g_vertexPerFace);
	}

	for (uint32_t iBlock = 0; iBlock < g_nBlocks; iBlock++)
	{
		auto iPos = chunk.blocks[iBlock].pos - chunk.pos;
		uint32_t iTop	= g_chunkSize.x * ((iPos.y + 1) * g_chunkSize.z + iPos.z) + iPos.x;
		uint32_t iFront = g_chunkSize.x * (iPos.y * g_chunkSize.z + (iPos.z + 1)) + iPos.x;
		uint32_t iRight = g_chunkSize.x * (iPos.y * g_chunkSize.z + iPos.z) + (iPos.x + 1);

		if (chunk.blocks[iBlock].type == BlockType::AIR)
		{
			bool topSolid	= iPos.y < g_chunkSize.y - 1 && chunk.blocks[iTop].type != BlockType::AIR;
			bool frontSolid = iPos.z < g_chunkSize.z - 1 && chunk.blocks[iFront].type != BlockType::AIR;
			bool rightSolid = iPos.x < g_chunkSize.x - 1 && chunk.blocks[iRight].type != BlockType::AIR;

			if (topSolid)	{ setBlockFace(chunk, iTop,	 Face::FaceType::BOTTOM); }
			if (frontSolid) { setBlockFace(chunk, iFront, Face::FaceType::BACK); }
			if (rightSolid) { setBlockFace(chunk, iRight, Face::FaceType::LEFT); }
		}
		else
		{
			bool topTrans	= iPos.y < g_chunkSize.y - 1 && chunk.blocks[iTop].type == BlockType::AIR;
			bool frontTrans = iPos.z < g_chunkSize.z - 1 && chunk.blocks[iFront].type == BlockType::AIR;
			bool rightTrans = iPos.x < g_chunkSize.x - 1 && chunk.blocks[iRight].type == BlockType::AIR;

			if (topTrans)	{ setBlockFace(chunk, iBlock, Face::FaceType::TOP); }
			if (frontTrans) { setBlockFace(chunk, iBlock, Face::FaceType::FRONT); }
			if (rightTrans) { setBlockFace(chunk, iBlock, Face::FaceType::RIGHT); }
		}
	}
	chunk.mesh.capacity = g_nBlocks * g_vertexPerCube * sizeof(Vertex);
	chunk.mesh.vertices.reserve(g_nBlocks * g_vertexPerCube);
}

void GameModule::generateMesh(Chunk& chunk)
{
	chunk.mesh.vertices.clear();
	
	std::sort(
		chunk.faces.begin(), chunk.faces.end(),
		[](const Face& face1, const Face& face2) {
			return face1.blockID < face2.blockID;
		}
	);
	
	for (const auto& face : chunk.faces)
	{
		chunk.mesh.vertices.insert(
			chunk.mesh.vertices.end(),
			face.vertices, face.vertices + g_vertexPerFace
		);
	}
}

void GameModule::loadChunkMesh(Chunk& chunk)
{
	loadData(&chunk.mesh);
}

void GameModule::drawChunk(const Chunk& chunk)
{
	if (chunk.mesh.vertices.size() == 0)
	{
		return;
	}

	renderMesh(&chunk.mesh);
}
