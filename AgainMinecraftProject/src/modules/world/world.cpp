#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "../../engine/ray/ray.h"
#include "../../engine/shader/shader.h"
#include "../../engine/renderer/block_renderer.h"

#include "../chunk/chunk.h"
#include "../player/player.h"

#include "world.h"

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr int32_t g_chunksX = 8;
constexpr int32_t g_chunksZ = 8;

void GameModule::initWorld(World& world)
{
	world.minBorder = glm::ivec3(0);
	world.maxBorder = glm::ivec3(g_chunkSize.x * g_chunksX, 0, g_chunkSize.z * g_chunksZ);

	for (int32_t z = 0; z < g_chunksZ; z++)
	{
		for (int32_t x = 0; x < g_chunksX; x++)
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

glm::ivec3 getChunkPos(const glm::vec3 pos) 
{
	return {
		static_cast<int32_t>(pos.x) / g_chunkSize.x * g_chunkSize.x,
		0,
		static_cast<int32_t>(pos.z) / g_chunkSize.z * g_chunkSize.z
	};
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
		currChunkPos = getChunkPos(currPos);

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

bool collAABB(const glm::vec3 player, const Block& block)
{
	if (block.type == BlockType::AIR)
	{
		return false;
	}

	bool collX = player.x + 1 > block.pos.x && block.pos.x + 1 > player.x;
	bool collZ = player.z + 1 > block.pos.z && block.pos.z + 1 > player.z;
	bool collY = player.y + 1 > block.pos.y && block.pos.y + 1 > player.y;
	return collX && collZ;//&& block.type != BlockType::AIR; //&& collY;
}

uint32_t getId(World& world, const glm::ivec3 pos)
{
	glm::ivec3 iPos = pos - getChunkPos(pos);

	return g_chunkSize.x * (g_chunkSize.z * iPos.y + iPos.z) + iPos.x;
}

Block& getBlock(World& world, const glm::vec3 pos)
{
	glm::ivec3 p = static_cast<glm::ivec3>(pos);

	return world.chunks[getChunkPos(pos)].blocks[getId(world, p)];
}

float sweptAABB(const Player& player, const Block& block, glm::ivec3& normals)
{
	float dxEntry, dzEntry;
	float dxExit, dzExit;

	glm::vec3 vel = 5.0f * glm::normalize(player.velocity);

	if (player.velocity.x > 0.0f)
	{
		dxEntry = block.pos.x - (player.pos.x + 1.0f);
		dxExit = block.pos.x + 1.0f - player.pos.x;
	}
	else
	{
		dxEntry = block.pos.x + 1.0f - player.pos.x;
		dxExit = block.pos.x - (player.pos.x + 1.0f);
	}

	if (player.velocity.z > 0.0f)
	{
		dzEntry = block.pos.z - (player.pos.z + 1.0f);
		dzExit = block.pos.z + 1.0f - player.pos.z;
	}
	else
	{
		dzEntry = block.pos.z + 1.0f - player.pos.z;
		dzExit = block.pos.z - (player.pos.z + 1.0f);
	}

	float xEntry, zEntry;
	float xExit, zExit;

	if (player.velocity.x == 0.0f)
	{
		xEntry = -std::numeric_limits<float>::infinity();
		xExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		xEntry = dxEntry / vel.x;
		xExit = dxExit / vel.x;
	}

	if (player.velocity.z == 0.0f)
	{
		zEntry = -std::numeric_limits<float>::infinity();
		zExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		zEntry = dzEntry / vel.z;
		zExit = dzExit / vel.z;
	}

	float entryTime = std::max(zEntry, xEntry);
	float exitTime = std::min(zExit, xExit);

	if (block.type == BlockType::AIR || 
		entryTime > exitTime || 
		xEntry < 0.0f && zEntry < 0.0f 
		|| xEntry > 1.0f || zEntry > 1.0f)
	{
		normals = { 0.0f, 0.0f, 0.0f };
		return 1.0f;
	}
	else
	{
		if (xEntry > zEntry)
		{
			if (dxEntry < 0.0f)
			{
				normals.x = 1;
				normals.z = 0;
			}
			else
			{
				normals.x = -1;
				normals.z = 0;
			}
		}
		else
		{
			if (dzEntry < 0.0f)
			{
				normals.x = 0;
				normals.z = 1;
			}
			else
			{
				normals.x = 0;
				normals.z = -1;
			}
		}

		return entryTime;
	}
}

void GameModule::checkPlayerCollision(World& world, Player& player, float dt)
{
	float xOffset = player.velocity.x > 0.0f ? 1.01f : -0.01f;
	float zOffset = player.velocity.z > 0.0f ? 1.01f : -0.01f;

	const glm::vec3 startPos = { player.pos.x, player.pos.y, player.pos.z };

	const glm::vec3 posZLeft	= { startPos.x,			  startPos.y, player.pos.z + zOffset };
	const glm::vec3 posZRight	= { startPos.x + 1.0f,	  startPos.y, player.pos.z + zOffset };
	const glm::vec3 posXBack	= { startPos.x + xOffset, startPos.y, player.pos.z };
	const glm::vec3 posXFront	= { startPos.x + xOffset, startPos.y, player.pos.z + 1.0f };

	const Block& leftZ	= getBlock(world, posZLeft);
	const Block& rightZ = getBlock(world, posZRight);
	const Block& backX	= getBlock(world, posXBack);
	const Block& frontX = getBlock(world, posXFront);

	glm::ivec3 normals;

	if (sweptAABB(player, leftZ, normals) < 1.0f || sweptAABB(player, rightZ, normals) < 1.0f)
	{
		player.pos.z -= player.velocity.z * dt;
		player.camera.pos.z -= player.velocity.z * dt;
	}

	if (sweptAABB(player, backX, normals) < 1.0f || sweptAABB(player, frontX, normals) < 1.0f)
	{
		player.pos.x -= player.velocity.x * dt;
		player.camera.pos.x -= player.velocity.x * dt;
	}
}