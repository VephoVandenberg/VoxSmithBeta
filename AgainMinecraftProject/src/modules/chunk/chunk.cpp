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

#include "../../engine/renderer/mesh.h"
#include "../../engine/ray/ray.h"

#include "block.h"
#include "chunk.h"

using namespace Engine::Renderer;

using namespace GameModule;

constexpr uint32_t g_chunkSizeX = 16;
constexpr uint32_t g_chunkSizeY = 16;
constexpr uint32_t g_chunkSizeZ = 16;

constexpr uint32_t g_nBlocks = g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ;

constexpr uint32_t g_vertexPerCube = 36;
constexpr uint32_t g_vertexPerFace = 6;

constexpr float g_rayDeltaMag = 0.25f;

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

Chunk GameModule::generateChunk(const glm::vec3 pos)
{
	Chunk chunk;
	chunk.pos = pos;
	if (!chunk.blocks)
	{
		chunk.blocks = new Block[g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ];
	}

	for (uint32_t x = 0; x < g_chunkSizeX; x++)
	{
		for (uint32_t y = 0; y < g_chunkSizeY; y++)
		{
			for (uint32_t z = 0; z < g_chunkSizeZ; z++)
			{
				uint32_t index = g_chunkSizeX * (y * g_chunkSizeZ + z) + x;
				chunk.blocks[index].type = y < 5 ? BlockType::GRASS_DIRT : BlockType::AIR;
				chunk.blocks[index].pos = pos + glm::vec3(x, y, z);
			}
		}
	}

	return chunk;
}

void GameModule::deleteChunk(Chunk& chunk)
{
	delete chunk.faces;
	delete chunk.blocks;
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

void updateFacePos(Vertex* vertices, const Block& block)
{
	for (uint32_t iVertex = 0; iVertex < g_vertexPerFace; iVertex++)
	{
		vertices[iVertex].pos += block.pos;
	}
}

void setBlockFace(Chunk& chunk, uint32_t id, const Vertex* face)
{
	Vertex vertices[g_vertexPerFace];
	std::copy(face, face + g_vertexPerFace, vertices);
	chunk.faces[chunk.nFaces].blockID = id;
	updateFacePos(vertices, chunk.blocks[id]);
	std::copy(
		vertices, vertices + g_vertexPerFace,
		chunk.faces[chunk.nFaces++].vertices
	);
}

void GameModule::initMeshData(Chunk& chunk)
{
	auto nBlocks = g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ;

	if (!chunk.faces)
	{
		chunk.faces = new Face[nBlocks * g_vertexPerFace];
	}

	chunk.nFaces = 0;
	for (uint32_t iBlock = 0; iBlock < nBlocks; iBlock++)
	{
		auto iPos = chunk.blocks[iBlock].pos - chunk.pos;
		uint32_t iTop = g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
		uint32_t iFront = g_chunkSizeX * (iPos.y * g_chunkSizeZ + (iPos.z + 1)) + iPos.x;
		uint32_t iRight = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + (iPos.x + 1);

		if (chunk.blocks[iBlock].type == BlockType::AIR)
		{
			if (iPos.y + 1 < g_chunkSizeY &&
				chunk.blocks[iTop].type != BlockType::AIR &&
				iPos.y < g_chunkSizeY - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::BOTTOM;
				setBlockFace(chunk, iTop, bottom);
			}

			if (chunk.blocks[iFront].type != BlockType::AIR &&
				iPos.z < g_chunkSizeZ - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::BACK;
				setBlockFace(chunk, iFront, back);
			}

			if (chunk.blocks[iRight].type != BlockType::AIR &&
				iPos.x < g_chunkSizeX - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::LEFT;
				setBlockFace(chunk, iRight, left);
			}
		}
		else
		{
			if (chunk.blocks[iTop].type == BlockType::AIR &&
				iPos.y < g_chunkSizeY - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::TOP;
				setBlockFace(chunk, iBlock, top);
			}

			if (chunk.blocks[iFront].type == BlockType::AIR &&
				iPos.z < g_chunkSizeZ - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::FRONT;
				setBlockFace(chunk, iBlock, front);
			}

			if (chunk.blocks[iRight].type == BlockType::AIR &&
				iPos.x < g_chunkSizeX - 1)
			{
				chunk.faces[chunk.nFaces].type = FaceType::RIGHT;
				setBlockFace(chunk, iBlock, right);
			}
		}
	}
	chunk.mesh.size = nBlocks * g_vertexPerCube;
}

void GameModule::generateMesh(Chunk& chunk)
{
	if (!chunk.mesh.vertices)
	{
		chunk.mesh.vertices = new Vertex[chunk.mesh.size];
	}
	else
	{
		memset(chunk.mesh.vertices, 0, chunk.mesh.size);
	}

	std::sort(
		chunk.faces,
		chunk.faces + chunk.nFaces,
		[](const Face& face1, const Face& face2) {
			return face1.blockID < face2.blockID;
		}
	);
	
	for (uint32_t iFace = 0; iFace < chunk.nFaces; iFace++)
	{
		std::copy(
			chunk.faces[iFace].vertices,
			chunk.faces[iFace].vertices + g_vertexPerFace,
			chunk.mesh.vertices + iFace * g_vertexPerFace
		);
	}
}

void GameModule::loadChunkMesh(Chunk& chunk)
{
	loadData(&chunk.mesh);
}

void GameModule::drawChunk(const Chunk& chunk)
{
	if (chunk.mesh.size == 0)
	{
		return;
	}

	renderMesh(&chunk.mesh);
}

void removeFace(Chunk& chunk, const Vertex* face, const uint32_t id)
{
	Vertex vertices[g_vertexPerFace];
	std::copy(
		face,
		face + g_vertexPerFace,
		vertices
	);
	updateFacePos(vertices, chunk.blocks[id]);
	std::remove_if(
		chunk.faces,
		chunk.faces + chunk.nFaces,
		[&](const Face& face) {
			if (id == face.blockID)
			{
				chunk.nFaces--;
				return true;
			}
			return false;
		}
	);
}

void addBlockFaces(Chunk& chunk, const glm::ivec3 iPos)
{
	uint32_t iTop = g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iBottom = g_chunkSizeX * ((iPos.y - 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iFront = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z + 1) + iPos.x;
	uint32_t iBack = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z - 1) + iPos.x;
	uint32_t iRight = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x + 1;
	uint32_t iLeft = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x - 1;
	uint32_t id = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x;

	if (iPos.y < g_chunkSizeY - 1)
	{
		if (chunk.blocks[iTop].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::TOP;
			setBlockFace(chunk, id, top);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iTop &&
						face.type == FaceType::BOTTOM)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
	if (iPos.y >= 0)
	{
		if (chunk.blocks[iBottom].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::BOTTOM;
			setBlockFace(chunk, id, bottom);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iBottom &&
						face.type == FaceType::TOP)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
	if (iPos.z < g_chunkSizeZ - 1)
	{
		if (chunk.blocks[iFront].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::FRONT;
			setBlockFace(chunk, id, front);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iFront &&
						face.type == FaceType::BACK)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
	if (iPos.z >= 0)
	{
		if (chunk.blocks[iBack].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::BACK;
			setBlockFace(chunk, id, back);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iBack &&
						face.type == FaceType::FRONT)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
	if (iPos.x < g_chunkSizeX - 1)
	{
		if (chunk.blocks[iRight].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::RIGHT;
			setBlockFace(chunk, id, right);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iRight &&
						face.type == FaceType::LEFT)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
	if (iPos.x >= 0)
	{
		if (chunk.blocks[iLeft].type == BlockType::AIR)
		{
			chunk.faces[chunk.nFaces].type = FaceType::LEFT;
			setBlockFace(chunk, id, left);
		}
		else
		{
			std::remove_if(
				chunk.faces,
				chunk.faces + g_nBlocks,
				[&](const Face& face) {
					if (face.blockID == iLeft &&
						face.type == FaceType::RIGHT)
					{
						chunk.nFaces--;
						return true;
					}
					return false;
				}
			);
		}
	}
}

void removeBlockFaces(Chunk& chunk, glm::ivec3 iPos)
{
	// Remove block faces
	uint32_t id = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x;
	std::remove_if(
		chunk.faces,
		chunk.faces + chunk.nFaces,
		[&](const Face& face) {
			if (face.blockID == id)
			{
				chunk.nFaces--;
				return true;
			}
			return false;
		}
	);

	// Add block's neighbours faces
	uint32_t iTop = g_chunkSizeX * ((iPos.y + 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iBottom = g_chunkSizeX * ((iPos.y - 1) * g_chunkSizeZ + iPos.z) + iPos.x;
	uint32_t iFront = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z + 1) + iPos.x;
	uint32_t iBack = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z - 1) + iPos.x;
	uint32_t iRight = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x + 1;
	uint32_t iLeft = g_chunkSizeX * (iPos.y * g_chunkSizeZ + iPos.z) + iPos.x - 1;

	if (iPos.y < g_chunkSizeY - 1 && 
		chunk.blocks[iTop].type != BlockType::AIR) 
	{ 
		chunk.faces[chunk.nFaces].type = FaceType::BOTTOM;
		setBlockFace(chunk, iTop, bottom); 
	}
	if (iPos.y >= 0 &&
		chunk.blocks[iBottom].type != BlockType::AIR) 
	{
		chunk.faces[chunk.nFaces].type = FaceType::TOP;
		setBlockFace(chunk, iBottom, top);
	}
	if (iPos.z < g_chunkSizeZ - 1 && 
		chunk.blocks[iFront].type != BlockType::AIR) 
	{ 
		chunk.faces[chunk.nFaces].type = FaceType::BACK;
		setBlockFace(chunk, iFront, back); 
	}
	if (iPos.z >= 0 && 
		chunk.blocks[iBack].type != BlockType::AIR) 
	{
		chunk.faces[chunk.nFaces].type = FaceType::FRONT;
		setBlockFace(chunk, iBack, front); 
	}
	if (iPos.x < g_chunkSizeX - 1 && 
		chunk.blocks[iRight].type != BlockType::AIR) 
	{
		chunk.faces[chunk.nFaces].type = FaceType::LEFT;
		setBlockFace(chunk, iRight, left); 
	}
	if (iPos.x >= 0 &&
		chunk.blocks[iLeft].type != BlockType::AIR) 
	{
		chunk.faces[chunk.nFaces].type = FaceType::RIGHT;
		setBlockFace(chunk, iLeft, right); 
	}
}

bool GameModule::processRayInChunk(Chunk& chunk, Engine::Ray& ray, RayType type)
{
	glm::vec3 dir = glm::normalize(ray.end - ray.start);

	auto rayLength = glm::length(ray.end - ray.start);
	auto nBlocks = g_chunkSizeX * g_chunkSizeY * g_chunkSizeZ;

	uint32_t x;
	uint32_t y;
	uint32_t z;
	glm::vec3 rayCurrPos;

	for (float rayFrac = 0; rayFrac < rayLength; rayFrac += g_rayDeltaMag)
	{
		rayCurrPos = ray.start + rayFrac * dir;
		glm::vec3 iPos = rayCurrPos - chunk.pos;

		x = static_cast<uint32_t>(iPos.x);
		y = static_cast<uint32_t>(iPos.y);
		z = static_cast<uint32_t>(iPos.z);

		uint32_t iBlock = g_chunkSizeX * (y * g_chunkSizeZ + z) + x;

		if (rayCurrPos.x < chunk.pos.x || rayCurrPos.x > chunk.pos.x + g_chunkSizeX ||
			rayCurrPos.z < chunk.pos.z || rayCurrPos.z > chunk.pos.z + g_chunkSizeZ)
		{
			continue;
		}

		if (chunk.blocks[iBlock].type != BlockType::AIR)
		{
			if (type == RayType::REMOVE)
			{
				chunk.blocks[iBlock].type = BlockType::AIR;
				removeBlockFaces(chunk, { x, y, z });
				ray.end = rayCurrPos;
				return true;
			}
			else
			{
				iPos -= g_rayDeltaMag * dir;
				x = static_cast<uint32_t>(iPos.x);
				y = static_cast<uint32_t>(iPos.y);
				z = static_cast<uint32_t>(iPos.z);
				iBlock = g_chunkSizeX * (y * g_chunkSizeZ + z) + x;
				chunk.blocks[iBlock].type = BlockType::GRASS_DIRT;
				ray.end = rayCurrPos;
				addBlockFaces(chunk, { x, y, z });
				return true;
			}
		}
	}

	if (type == RayType::PLACE)
	{
		if (rayCurrPos.x < chunk.pos.x || rayCurrPos.x > chunk.pos.x + g_chunkSizeX ||
			rayCurrPos.z < chunk.pos.z || rayCurrPos.z > chunk.pos.z + g_chunkSizeZ)
		{
			return false;
		}

		uint32_t iBlock = g_chunkSizeX * (y * g_chunkSizeZ + z) + x;
		chunk.blocks[iBlock].type = BlockType::GRASS_DIRT;
		addBlockFaces(chunk, {x, y, z});
		return true;
	}

	return false;
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
					more.faces[more.nFaces].type = FaceType::LEFT;
					setBlockFace(more, iPos2, left);
				}
				else if (less.blocks[iPos1].type != BlockType::AIR &&
					more.blocks[iPos2].type == BlockType::AIR)
				{
					less.faces[more.nFaces].type = FaceType::RIGHT;
					setBlockFace(less, iPos1, right);
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
					more.faces[more.nFaces].type = FaceType::BACK;
					setBlockFace(more, iPos2, back);
				}
				else if (less.blocks[iPos1].type != BlockType::AIR &&
					more.blocks[iPos2].type == BlockType::AIR)
				{
					less.faces[less.nFaces].type = FaceType::FRONT;
					setBlockFace(less, iPos1, front);
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