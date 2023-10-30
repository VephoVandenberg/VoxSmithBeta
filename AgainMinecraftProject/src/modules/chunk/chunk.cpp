/**
* Steps to draw the chunks and
* check and update neighbour chunks if needed.
*
* 1. Generate blocks.
* 2. Init mesh data,
*	 that means we are loading up faces
*	 to the array with the correspoding vertices, also we store the block ID(index),
*	 and type of face that will be drawn.
* 3. (This step might not be needed because we will move the chunk generation into the world struct)
	  Check the neighbour chunks if there are block faces that need to be rendered.
* 4. Sort the faces array by block id, then generate the mesh.
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

constexpr uint32_t g_chunkSizeX = 16;
constexpr uint32_t g_chunkSizeY = 256;
constexpr uint32_t g_chunkSizeZ = 16;

constexpr uint32_t g_nBlocks = g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ;

constexpr uint32_t g_vertexPerCube = 36;
constexpr uint32_t g_vertexPerFace = 6;

constexpr float g_rayDeltaMag = 0.1f;

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

BlockType getType()
{
	return BlockType::GRASS;
}

Chunk GameModule::generateChunk(const glm::ivec3 pos)
{
	Chunk chunk;
	chunk.pos = pos;
	if (chunk.blocks.empty())
	{
		chunk.blocks.reserve(g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ);
	}

	for (int32_t y = 0; y < g_chunkSizeY; y++)
	{
		for (int32_t z = 0; z < g_chunkSizeZ; z++)
		{
			for (int32_t x = 0; x < g_chunkSizeX; x++)
			{
				Block block;
				block.pos = chunk.pos + glm::vec3(x, y, z);
				if (y < g_chunkSizeY / 2)
				{
					block.type = BlockType::DIRT;
				}
				else if (y == g_chunkSizeY / 2)
				{
					block.type = BlockType::GRASS;
				}
				else
				{
					block.type = BlockType::AIR;
				}

				switch (block.type)
				{
				case BlockType::GRASS:
					block.top		= 0;
					block.side		= 1;
					block.bottom	= 2;
					break;

				case BlockType::DIRT:
					block.top = block.side = block.bottom = 2;
					break;
				}

				chunk.blocks.push_back(block);
			}
		}
	}

	return chunk;
}

void updateFacePos(Vertex* vertices, const Block& block)
{
	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		vertices[iVertex].pos += block.pos;
	}
}

void GameModule::setBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	chunk.updated = false;
	Vertex vertices[g_vertexPerFace];
	std::copy(g_faces[type].begin(), g_faces[type].end(), vertices);
	updateFacePos(vertices, chunk.blocks[id]);

	Face face_;
	face_.type = type;
	face_.blockID = id;

	uint32_t texID;

	switch (type)
	{
	case Face::FaceType::TOP:
		texID = chunk.blocks[id].top;
		break;

	case Face::FaceType::BOTTOM:
		texID = chunk.blocks[id].bottom;
		break;

	default:
		texID = chunk.blocks[id].side;
		break;
	}

	for (auto& vert : vertices)
	{
		vert.uvw.z = texID;
	}

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
		uint32_t iTop	= g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
		uint32_t iFront = g_chunkSizeX * (iPos.y * g_chunkSizeZ + (iPos.z + 1)) + iPos.x;
		uint32_t iRight = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + (iPos.x + 1);

		if (chunk.blocks[iBlock].type == BlockType::AIR)
		{
			bool topSolid	= iPos.y < g_chunkSizeY - 1 && chunk.blocks[iTop].type != BlockType::AIR;
			bool frontSolid = iPos.z < g_chunkSizeZ - 1 && chunk.blocks[iFront].type != BlockType::AIR;
			bool rightSolid = iPos.x < g_chunkSizeX - 1 && chunk.blocks[iRight].type != BlockType::AIR;

			if (topSolid)	{ setBlockFace(chunk, iTop,	 Face::FaceType::BOTTOM); }
			if (frontSolid) { setBlockFace(chunk, iFront, Face::FaceType::BACK); }
			if (rightSolid) { setBlockFace(chunk, iRight, Face::FaceType::LEFT); }
		}
		else
		{
			bool topTrans	= iPos.y < g_chunkSizeY - 1 && chunk.blocks[iTop].type == BlockType::AIR;
			bool frontTrans = iPos.z < g_chunkSizeZ - 1 && chunk.blocks[iFront].type == BlockType::AIR;
			bool rightTrans = iPos.x < g_chunkSizeX - 1 && chunk.blocks[iRight].type == BlockType::AIR;

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
