/**
* Steps to draw the chunks and
* check and update neighbour chunks if needed.
*
* 1. Generate blocks.
* 2. Init mesh data,
*	 that means we are loading up faces
*	 to the array with the correspoding vertices, also we store the block ID(index),
*	 and type of face that will be drawn.
* 3. Check the neighbour chunks if there are block faces that need to be rendered.
* 4. Sort the faces array by block id, then generate the mesh.
*/


#include <algorithm>
#include <glm/glm.hpp>
#include <iostream>

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

constexpr Vertex back[] = {
	{{ 0, 0, 0 }, { 0, 0 }},
	{{ 0, 1, 0 }, { 0, 1 }},
	{{ 1, 0, 0 }, { 1, 0 }},

	{{ 1, 0, 0 }, { 1, 0 }},
	{{ 0, 1, 0 }, { 0, 1 }},
	{{ 1, 1, 0 }, { 1, 1 }},
};

constexpr Vertex front[] = {
	{{ 0, 0, 1 }, { 0, 0 }},
	{{ 1, 0, 1 }, { 1, 0 }},
	{{ 0, 1, 1 }, { 0, 1 }},

	{{ 1, 0, 1 }, { 1, 0 }},
	{{ 1, 1, 1 }, { 1, 1 }},
	{{ 0, 1, 1 }, { 0, 1 }},
};

constexpr Vertex top[] = {
	{{ 0, 1, 1 }, { 0, 0 }},
	{{ 1, 1, 1 }, { 1, 0 }},
	{{ 0, 1, 0 }, { 0, 1 }},

	{{ 1, 1, 1 }, { 1, 0 }},
	{{ 1, 1, 0 }, { 1, 1 }},
	{{ 0, 1, 0 }, { 0, 1 }},
};

constexpr Vertex bottom[] = {
	{{ 0, 0, 1 }, { 0, 0 }},
	{{ 0, 0, 0 }, { 0, 1 }},
	{{ 1, 0, 1 }, { 1, 0 }},

	{{ 1, 0, 0 }, { 1, 1 }},
	{{ 1, 0, 1 }, { 1, 0 }},
	{{ 0, 0, 0 }, { 0, 1 }},
};

constexpr Vertex left[] = {
	{{ 0, 0, 0 }, { 0, 0 }},
	{{ 0, 0, 1 }, { 1, 0 }},
	{{ 0, 1, 0 }, { 0, 1 }},

	{{ 0, 0, 1 }, { 1, 0 }},
	{{ 0, 1, 1 }, { 1, 1 }},
	{{ 0, 1, 0 }, { 0, 1 }},
};

constexpr Vertex right[] = {
	{{ 1, 0, 1 }, { 0, 0 }},
	{{ 1, 0, 0 }, { 1, 0 }},
	{{ 1, 1, 1 }, { 0, 1 }},

	{{ 1, 0, 0 }, { 1, 0 }},
	{{ 1, 1, 0 }, { 1, 1 }},
	{{ 1, 1, 1 }, { 0, 1 }}
};

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
				chunk.blocks.push_back({
					y < g_chunkSizeY / 2 ? BlockType::GRASS_DIRT : BlockType::AIR,
					chunk.pos + glm::vec3(x, y, z)
					});
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

void GameModule::setBlockFace(Chunk& chunk, uint32_t id, const Vertex* face, Face::FaceType type)
{
	Vertex vertices[g_vertexPerFace];
	std::copy(face, face + g_vertexPerFace, vertices);
	updateFacePos(vertices, chunk.blocks[id]);

	Face face_;
	face_.type = type;
	face_.blockID = id;

	std::copy(vertices, vertices + g_vertexPerFace, face_.vertices);
	chunk.faces.push_back(face_);
}

void GameModule::removeBlockFace(Chunk& chunk, uint32_t id, Face::FaceType type)
{
	auto itDel = std::remove_if(
		chunk.faces.begin(), chunk.faces.end(),
		[&](const Face& face) {
			return face.blockID == id && face.type == type;
		}
	);
	chunk.faces.erase(itDel, chunk.faces.end());
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

			if (topSolid)	{ setBlockFace(chunk, iTop, bottom, Face::FaceType::BOTTOM); }
			if (frontSolid) { setBlockFace(chunk, iFront, back, Face::FaceType::BACK); }
			if (rightSolid) { setBlockFace(chunk, iRight, left, Face::FaceType::LEFT); }
		}
		else
		{
			bool topTrans	= iPos.y < g_chunkSizeY - 1 && chunk.blocks[iTop].type == BlockType::AIR;
			bool frontTrans = iPos.z < g_chunkSizeZ - 1 && chunk.blocks[iFront].type == BlockType::AIR;
			bool rightTrans = iPos.x < g_chunkSizeX - 1 && chunk.blocks[iRight].type == BlockType::AIR;

			if (topTrans)	{ setBlockFace(chunk, iBlock, top, Face::FaceType::TOP); }
			if (frontTrans) { setBlockFace(chunk, iBlock, front, Face::FaceType::FRONT); }
			if (rightTrans) { setBlockFace(chunk, iBlock, right, Face::FaceType::RIGHT); }
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

void GameModule::addBlock(Chunk& chunk, const glm::ivec3 iPos)
{
	uint32_t iTop		= g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iBottom	= g_chunkSizeX * ((iPos.y - 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iFront		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z + 1) + iPos.x;
	uint32_t iBack		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z - 1) + iPos.x;
	uint32_t iRight		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x + 1;
	uint32_t iLeft		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x - 1;
	uint32_t id			= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x;

	if (iPos.y < g_chunkSizeY - 1)
	{
		if (chunk.blocks[iTop].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, top, Face::FaceType::TOP);
		}
		else
		{
			removeBlockFace(chunk, iTop, Face::FaceType::BOTTOM);			
		}
	}
	if (iPos.y >= 0)
	{
		if (chunk.blocks[iBottom].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, bottom, Face::FaceType::BOTTOM);
		}
		else
		{
			removeBlockFace(chunk, iBottom, Face::FaceType::TOP);
		}
	}
	if (iPos.z < g_chunkSizeZ - 1)
	{
		if (chunk.blocks[iFront].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, front, Face::FaceType::FRONT);
		}
		else
		{
			removeBlockFace(chunk, iFront, Face::FaceType::BACK);
		}
	}
	if (iPos.z >= 0)
	{
		if (chunk.blocks[iBack].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, back, Face::FaceType::BACK);
		}
		else
		{
			removeBlockFace(chunk, iBack, Face::FaceType::FRONT);
		}
	}
	if (iPos.x < g_chunkSizeX - 1)
	{
		if (chunk.blocks[iRight].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, right, Face::FaceType::RIGHT);
		}
		else
		{
			removeBlockFace(chunk, iRight, Face::FaceType::LEFT);
		}
	}
	if (iPos.x >= 0)
	{
		if (chunk.blocks[iLeft].type == BlockType::AIR)
		{
			setBlockFace(chunk, id, left, Face::FaceType::LEFT);
		}
		else
		{
			removeBlockFace(chunk, iLeft, Face::FaceType::RIGHT);
		}
	}
}

void GameModule::removeBlock(Chunk& chunk, glm::ivec3 iPos)
{
	// Remove block faces
	uint32_t id = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x;
	
	auto deleteStart = std::remove_if(
		chunk.faces.begin(), chunk.faces.end(),
		[&](Face& face) {
			return face.blockID == id;
		}
	);
	chunk.faces.erase(deleteStart, chunk.faces.end());
	
	// Add block's neighbours faces
	uint32_t iTop		= g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iBottom	= g_chunkSizeX * ((iPos.y - 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iFront		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z + 1) + iPos.x;
	uint32_t iBack		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z - 1) + iPos.x;
	uint32_t iRight		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x + 1;
	uint32_t iLeft		= g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x - 1;

	bool topSolid		= iPos.y < g_chunkSizeY - 1 && chunk.blocks[iTop].type != BlockType::AIR;
	bool bottomSolid	= iPos.y >= 0 && chunk.blocks[iBottom].type != BlockType::AIR;
	bool frontSolid		= iPos.z < g_chunkSizeZ - 1 && chunk.blocks[iFront].type != BlockType::AIR;
	bool backSolid		= iPos.z >= 0 && chunk.blocks[iBack].type != BlockType::AIR;
	bool rightSolid		= iPos.x < g_chunkSizeX - 1 && chunk.blocks[iRight].type != BlockType::AIR;
	bool leftSolid		= iPos.x >= 0 && chunk.blocks[iLeft].type != BlockType::AIR;

	if (topSolid)		{ setBlockFace(chunk, iTop, bottom, Face::FaceType::BOTTOM); }
	if (bottomSolid)	{ setBlockFace(chunk, iBottom, top, Face::FaceType::TOP); }
	if (frontSolid)		{ setBlockFace(chunk, iFront, back, Face::FaceType::BACK); }
	if (backSolid)		{ setBlockFace(chunk, iBack, front, Face::FaceType::FRONT); }
	if (rightSolid)		{ setBlockFace(chunk, iRight, left, Face::FaceType::LEFT); }
	if (leftSolid)		{ setBlockFace(chunk, iLeft, right, Face::FaceType::RIGHT); }
}

void GameModule::updateChunkNeighbourFace(Chunk& less, Chunk& more)
{
	if (less.pos.x < more.pos.x)
	{
		for (uint32_t y = 0; y < g_chunkSizeY; y++)
		{
			for (uint32_t z = 0; z < g_chunkSizeZ; z++)
			{
				uint32_t iPos1 = g_chunkSizeX * (y * g_chunkSizeZ + z) + g_chunkSizeX - 1;
				uint32_t iPos2 = g_chunkSizeX * (y * g_chunkSizeZ + z) + 0;

				if (less.blocks[iPos1].type == BlockType::AIR &&
					more.blocks[iPos2].type != BlockType::AIR)
				{
					setBlockFace(more, iPos2, left, Face::FaceType::LEFT);
				}
				else if (less.blocks[iPos1].type != BlockType::AIR &&
					more.blocks[iPos2].type == BlockType::AIR)
				{
					setBlockFace(less, iPos1, right, Face::FaceType::RIGHT);
				}
			}
		}
	}
	else if (less.pos.z < more.pos.z)
	{
		for (uint32_t y = 0; y < g_chunkSizeY; y++)
		{
			for (uint32_t x = 0; x < g_chunkSizeX; x++)
			{
				uint32_t iPos1 = g_chunkSizeX * (y * g_chunkSizeZ + (g_chunkSizeZ - 1)) + x;
				uint32_t iPos2 = g_chunkSizeX * (y * g_chunkSizeZ + (0)) + x;

				if (less.blocks[iPos1].type == BlockType::AIR &&
					more.blocks[iPos2].type != BlockType::AIR)
				{
					setBlockFace(more, iPos2, back, Face::FaceType::BACK);
				}
				else if (less.blocks[iPos1].type != BlockType::AIR &&
					more.blocks[iPos2].type == BlockType::AIR)
				{
					setBlockFace(less, iPos1, front, Face::FaceType::FRONT);
				}
			}
		}
	}
}

bool GameModule::rayStartInChunk(const Chunk& chunk, const Engine::Ray& ray)
{
	return
		chunk.pos.x <= ray.start.x && ray.start.x <= chunk.pos.x + g_chunkSizeX &&
		chunk.pos.z <= ray.start.z && ray.start.z <= chunk.pos.z + g_chunkSizeZ;
}

bool GameModule::rayEndInChunk(const Chunk& chunk, const Engine::Ray& ray)
{
	return
		chunk.pos.x <= ray.end.x && ray.end.x <= chunk.pos.x + g_chunkSizeX &&
		chunk.pos.z <= ray.end.z && ray.end.z <= chunk.pos.z + g_chunkSizeZ;
}

bool GameModule::rayEndInBorderX(const Chunk& chunk, const Engine::Ray& ray)
{
	return
		(chunk.pos.x + 1 >= ray.end.x && chunk.pos.x <= ray.end.x) ||
		(chunk.pos.x + g_chunkSizeX - 1 <= ray.end.x && chunk.pos.x + g_chunkSizeX >= ray.end.x);
}

bool GameModule::rayEndInBorderZ(const Chunk& chunk, const Engine::Ray& ray)
{
	return
		(chunk.pos.z + 1 >= ray.end.z && chunk.pos.z <= ray.end.z) ||
		(chunk.pos.z + g_chunkSizeZ - 1 <= ray.end.z && chunk.pos.z + g_chunkSizeZ >= ray.end.z);
}

bool GameModule::rayEndInBorderXPos(const Chunk& chunk, const Engine::Ray& ray)
{
	return (chunk.pos.x + g_chunkSizeX - 1 <= ray.end.x && chunk.pos.x + g_chunkSizeX >= ray.end.x);
}

bool GameModule::rayEndInBorderZPos(const Chunk& chunk, const Engine::Ray& ray)
{
	return (chunk.pos.z + g_chunkSizeZ - 1 <= ray.end.z && chunk.pos.z + g_chunkSizeZ >= ray.end.z);
}

bool GameModule::rayEndInBorderXNeg(const Chunk& chunk, const Engine::Ray& ray)
{
	return (chunk.pos.x + 1 >= ray.end.x && chunk.pos.x <= ray.end.x);
}

bool GameModule::rayEndInBorderZNeg(const Chunk& chunk, const Engine::Ray& ray)
{
	return (chunk.pos.z + 1 >= ray.end.z && chunk.pos.z <= ray.end.z);
}
