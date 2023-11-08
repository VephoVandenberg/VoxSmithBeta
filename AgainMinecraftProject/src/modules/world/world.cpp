#include <glm/gtc/matrix_transform.hpp>

#include "../../engine/ray/ray.h"
#include "../../engine/shader/shader.h"
#include "../../engine/renderer/block_renderer.h"

#include "../chunk/chunk.h"

#include "world.h"

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr int32_t g_chunksX = 8;
constexpr int32_t g_chunksZ = 8;

void GameModule::initWorld(World& world)
{
	world.minBorder = glm::ivec3(0);
	world.maxBorder = glm::ivec3(g_chunkSize.x * g_chunksX, 0, g_chunkSize.z * g_chunksZ);

	for (int32_t z = 0;
		z < g_chunksZ;
		z++)
	{
		for (int32_t x = 0;
			x < g_chunksX;
			x++)
		{
			glm::ivec3 chunkPos = { x * g_chunkSize.x, 0, z * g_chunkSize.z};
			world.chunks[chunkPos] = generateChunk(chunkPos);
			initMeshData(world.chunks[chunkPos]);
			world.chunks[chunkPos].updated = false;
		}
	}
}

void GameModule::drawWorld(World& world)
{
	for (auto& pair : world.chunks)
	{
		if (!pair.second.updated)
		{
			generateMesh(pair.second);
			loadChunkMesh(pair.second);
			pair.second.updated = true;
		}
		drawChunk(pair.second);
	}
}

void GameModule::processRay(World& world, Engine::Ray& ray, Engine::Shader& shader, RayType type)
{
	bool		hit				= false;
	float		dl				= 0.1f;
	float		length			= glm::length(ray.end - ray.start);
	
	glm::vec3	dir				= glm::normalize(ray.end - ray.start);
	glm::vec3	currPos			= {};
	
	glm::ivec3	iPos			= glm::ivec3(0);
	glm::ivec3	currChunkPos	= {};

	for (float mag = 0.0f; mag < length; mag += dl)
	{
		currPos = ray.start + mag * dir;
		currChunkPos = {
			static_cast<int32_t>(currPos.x) / g_chunkSize.x * g_chunkSize.x,
			0,
			static_cast<int32_t>(currPos.z) / g_chunkSize.z * g_chunkSize.z
		};

		if (world.chunks.find(currChunkPos) == world.chunks.end())
		{
			continue;
		}

		iPos		= static_cast<glm::ivec3>(currPos) - currChunkPos;
		uint32_t id = g_chunkSize.x * (g_chunkSize.x * iPos.y + iPos.z) + iPos.x;

		if (world.chunks[currChunkPos].blocks[id].type != BlockType::AIR)
		{
			if (type == RayType::PLACE)
			{
				currPos -= dl * dir;
				iPos = static_cast<glm::ivec3>(currPos) - currChunkPos;
			}

			traceRay(world, currPos, shader, type);
			hit = true;
			break;
		}
	}

	if (world.chunks.find(currChunkPos) != world.chunks.end())
	{
		if ((type == RayType::PLACE) && !hit)
		{
			uint32_t id = g_chunkSize.x * (iPos.y * g_chunkSize.z + iPos.z) + iPos.x;
			world.chunks[currChunkPos].blocks[id].type = BlockType::DIRT;
			traceRay(world, currPos, shader, type);
		}
	}

	if (type == RayType::IDLE)
	{
		glm::vec3 pos = static_cast<glm::ivec3>(currPos);
		Engine::setUniform3f(shader, "u_position", pos);
	}
}

void GameModule::traceRay(World& world, glm::vec3 rayPosFrac, Engine::Shader& shader, GameModule::RayType type)
{
	auto getChunkPos = [](const glm::vec3 pos) {
		return glm::ivec3(
			static_cast<int32_t>(pos.x) / g_chunkSize.x * g_chunkSize.x,
			0,
			static_cast<int32_t>(pos.z) / g_chunkSize.z * g_chunkSize.z
		);
	};

	glm::ivec3 currBlock	= static_cast<glm::ivec3>(rayPosFrac);

	glm::ivec3 rightBlock	= { currBlock.x + 1,	currBlock.y,		currBlock.z };
	glm::ivec3 leftBlock	= { currBlock.x - 1,	currBlock.y,		currBlock.z };
	glm::ivec3 frontBlock	= { currBlock.x,		currBlock.y,		currBlock.z + 1 };
	glm::ivec3 backBlock	= { currBlock.x,		currBlock.y,		currBlock.z - 1 };

	glm::ivec3 iPosCurr		= currBlock		- getChunkPos(currBlock);
	glm::ivec3 iPosRight	= rightBlock	- getChunkPos(rightBlock);
	glm::ivec3 iPosLeft		= leftBlock		- getChunkPos(leftBlock);
	glm::ivec3 iPosFront	= frontBlock	- getChunkPos(frontBlock);
	glm::ivec3 iPosBack		= backBlock		- getChunkPos(backBlock);

	uint32_t iCurr			= g_chunkSize.x * (g_chunkSize.z * iPosCurr.y		+ iPosCurr.z) + iPosCurr.x;
	uint32_t iTop			= g_chunkSize.x * (g_chunkSize.z * (iPosCurr.y + 1) + iPosCurr.z) + iPosCurr.x;
	uint32_t iBottom		= g_chunkSize.x * (g_chunkSize.z * (iPosCurr.y - 1) + iPosCurr.z) + iPosCurr.x;

	uint32_t iRight			= g_chunkSize.x * (g_chunkSize.z * iPosRight.y	+ iPosRight.z)	+ iPosRight.x;
	uint32_t iLeft			= g_chunkSize.x * (g_chunkSize.z * iPosLeft.y	+ iPosLeft.z)	+ iPosLeft.x;
	uint32_t iFront			= g_chunkSize.x * (g_chunkSize.z * iPosFront.y	+ iPosFront.z)	+ iPosFront.x;
	uint32_t iBack			= g_chunkSize.x * (g_chunkSize.z * iPosBack.y	+ iPosBack.z)	+ iPosBack.x;

	bool rightSolid			= world.chunks[getChunkPos(rightBlock)].blocks[iRight].type	!= BlockType::AIR;
	bool leftSolid			= world.chunks[getChunkPos(leftBlock)].blocks[iLeft].type	!= BlockType::AIR;
	bool topSolid			= world.chunks[getChunkPos(currBlock)].blocks[iTop].type	!= BlockType::AIR;
	bool bottomSolid		= world.chunks[getChunkPos(currBlock)].blocks[iBottom].type != BlockType::AIR;
	bool frontSolid			= world.chunks[getChunkPos(frontBlock)].blocks[iFront].type != BlockType::AIR;
	bool backSolid			= world.chunks[getChunkPos(backBlock)].blocks[iBack].type	!= BlockType::AIR;

	if (type == RayType::REMOVE)
	{
		auto& curr = world.chunks[getChunkPos(currBlock)];
		world.chunks[getChunkPos(currBlock)].blocks[iCurr].type = BlockType::AIR;

		curr.faces.erase(
			std::remove_if(
				curr.faces.begin(), curr.faces.end(),
				[&](const Face& face) {
					return face.blockID == iCurr;
				}),
			curr.faces.end()
		);
		curr.updated = false;

		if (rightSolid)		{ setBlockFace(world.chunks[getChunkPos(rightBlock)],	iRight,		Face::FaceType::LEFT); }
		if (leftSolid)		{ setBlockFace(world.chunks[getChunkPos(leftBlock)],	iLeft,		Face::FaceType::RIGHT); }
		if (topSolid)		{ setBlockFace(world.chunks[getChunkPos(currBlock)],	iTop,		Face::FaceType::BOTTOM); }
		if (bottomSolid)	{ setBlockFace(world.chunks[getChunkPos(currBlock)],	iBottom,	Face::FaceType::TOP); }
		if (frontSolid)		{ setBlockFace(world.chunks[getChunkPos(frontBlock)],	iFront,		Face::FaceType::BACK); }
		if (backSolid)		{ setBlockFace(world.chunks[getChunkPos(backBlock)],	iBack,		Face::FaceType::FRONT); }
	}
	else if (type == RayType::PLACE)
	{
		auto& block = world.chunks[getChunkPos(currBlock)].blocks[iCurr];
		block.type = BlockType::DIRT;
		setType(block);

		if (rightSolid) { removeBlockFace(world.chunks[getChunkPos(rightBlock)], iRight, Face::FaceType::LEFT); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::RIGHT); }

		if (leftSolid) { removeBlockFace(world.chunks[getChunkPos(leftBlock)], iLeft, Face::FaceType::RIGHT); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::LEFT); }

		if (topSolid) { removeBlockFace(world.chunks[getChunkPos(currBlock)], iTop, Face::FaceType::BOTTOM); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::TOP); }

		if (bottomSolid) { removeBlockFace(world.chunks[getChunkPos(currBlock)], iBottom, Face::FaceType::TOP); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::BOTTOM); }

		if (frontSolid) { removeBlockFace(world.chunks[getChunkPos(frontBlock)], iFront, Face::FaceType::BACK); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::FRONT); }

		if (backSolid) { removeBlockFace(world.chunks[getChunkPos(backBlock)], iBack, Face::FaceType::FRONT); }
		else { setBlockFace(world.chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::BACK); }
	}
	else
	{
		glm::vec3 pos = static_cast<glm::ivec3>(rayPosFrac);
		Engine::setUniform3f(shader, "u_position", pos);
	}
}
