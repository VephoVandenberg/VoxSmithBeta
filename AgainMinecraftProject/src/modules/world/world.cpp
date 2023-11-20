#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <array>

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
			generateMesh(world.chunks[chunkPos]);
			loadChunkMesh(world.chunks[chunkPos]);
			world.chunks[chunkPos].updated = true;
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
	return collX && collY && collZ;
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

bool sweptAABB(const Player& player, const Block& block, glm::vec3& normals, float& hitTime)
{
	float dxEntry, dzEntry, dyEntry;
	float dxExit, dzExit, dyExit;

	glm::vec3 dir = player.velocity;

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

	if (player.velocity.y > 0.0f)
	{
		dyEntry = block.pos.y - (player.pos.y + 1.0f);
		dyExit = block.pos.y + 1.0f - player.pos.y;
	}
	else
	{
		dyEntry = block.pos.y + 1.0f - player.pos.y;
		dyExit = block.pos.y - (player.pos.y + 1.0f);
	}

	float xEntry, zEntry, yEntry;
	float xExit, zExit, yExit;

	if (player.velocity.x == 0.0f)
	{
		xEntry = -std::numeric_limits<float>::infinity();
		xExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		xEntry = dxEntry / dir.x;
		xExit = dxExit / dir.x;
	}

	if (player.velocity.z == 0.0f)
	{
		zEntry = -std::numeric_limits<float>::infinity();
		zExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		zEntry = dzEntry / dir.z;
		zExit = dzExit / dir.z;
	}

	if (player.velocity.y == 0.0f)
	{
		yEntry = -std::numeric_limits<float>::infinity();
		yExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		yEntry = dyEntry / dir.y;
		yExit = dyExit / dir.y;
	}

	hitTime			= std::max(std::max(xEntry, zEntry), yEntry);
	float exitTime	= std::min(std::min(xExit, zExit), yExit);

	if (block.type == BlockType::AIR || 
		hitTime > exitTime ||
		(xEntry < 0.0f && zEntry < 0.0f && yEntry < 0.0f)
		|| xEntry > 1.0f || zEntry > 1.0f || yEntry > 1.0f)
	{
		return false;
	}

	if (xEntry > yEntry)
	{
		if (xEntry > zEntry)
		{
			if (dxEntry < 0.0f)
			{
				normals = { 1, 0, 0 };
			}
			else
			{
				normals = { -1, 0, 0 };
			}
		}
		else
		{
			if (dzEntry < 0.0f)
			{
				normals = { 0, 0, 1 };
			}
			else
			{
				normals = { 0, 0, -1 };
			}
		}
	}
	else
	{
		if (yEntry > zEntry)
		{
			if (dyEntry < 0.0f)
			{
				normals = { 0, 1, 0 };
			}
			else
			{
				normals = { 0, -1, 0 };
			}
		}
		else
		{
			if (dzEntry < 0.0f)
			{
				normals = { 0, 0, 1 };
			}
			else
			{
				normals = { 0, 0, -1 };
			}
		}
	}
	return true;
}


bool rayVsCube(const Engine::Ray& ray, const Block& block, const glm::vec3& size, glm::vec3 normals, float& tHitNear)
{
	glm::vec3 dir = ray.end - ray.start;

	glm::vec3 tNear = (block.pos - ray.start) / dir;
	glm::vec3 tFar = (block.pos + size - ray.start) / dir;
	
	if (tNear.x > tFar.x) std::swap(tNear.x, tFar.x);
	if (tNear.z > tFar.z) std::swap(tNear.z, tFar.z);

	if (tNear.x > tFar.z || tNear.z > tFar.x) return false;

	tHitNear = std::max(tNear.x, tNear.z);
	float tHitFar = std::min(tFar.x, tFar.z);

	if (tHitFar < 0 || tHitNear > 1) return false;

	glm::vec3 contact = ray.start + tHitNear * dir;

	if (tNear.x > tNear.z)
	{
		if (dir.x < 0)
		{
			normals = { 1, 0, 0 };
		}
		else
		{
			normals = { -1, 0, 0 };
		}
	}
	else if (tNear.x < tNear.z)
	{
		if (dir.z < 0)
		{
			normals = { 0, 0, 1 };
		}
		else
		{
			normals = { 0, 0, -1 };
		}
	}

	return true;
}

bool playerVsCube(const Player& player, const Block& block, glm::vec3& normals)
{
	Block updatedBlock = block;

	updatedBlock.pos = block.pos - glm::vec3(0.5f);

	Engine::Ray ray;
	ray.start = player.pos + glm::vec3(0.5f);
	ray.end = ray.start + player.velocity;
	float time;
	if (rayVsCube(ray, updatedBlock, glm::vec3(1.5f), normals, time))
	{
		return true;
	}

	return false;
}


void GameModule::checkPlayerCollision(World& world, Player& player, float dt)
{
	float x = player.velocity.x > 0.0f ? 2.0f : -1.0f;
	float z = player.velocity.z > 0.0f ? 2.0f : -1.0f;
	float y = player.velocity.y > 0.0f ? 3.0f : -1.0f;

	const glm::vec3 start = { player.pos.x, player.pos.y, player.pos.z };

	const Block& leftZ = getBlock(world, { start.x,			start.y,		start.z + z });
	const Block& rightZ = getBlock(world, { start.x + 1.0f,		start.y,		start.z + z });
	const Block& backX = getBlock(world, { start.x + x,		start.y,		start.z });
	const Block& frontX = getBlock(world, { start.x + x,		start.y,		start.z + 1.0f });
	const Block& leftFZ = getBlock(world, { start.x,			start.y + 1.0f,	start.z + z });
	const Block& rightFZ = getBlock(world, { start.x + 1.0f,		start.y + 1.0f,	start.z + z });
	const Block& backFX = getBlock(world, { start.x + x,		start.y + 1.0f,	start.z });
	const Block& frontFX = getBlock(world, { start.x + x,		start.y + 1.0f,	start.z + 1.0f });
	const Block& frontLY = getBlock(world, { start.x,			start.y + y,	start.z + 1.0f });
	const Block& frontRY = getBlock(world, { start.x + 1.0f,		start.y + y,	start.z + 1.0f });
	const Block& backLY = getBlock(world, { start.x,			start.y + y,	start.z });
	const Block& backRY = getBlock(world, { start.x + 1.0f,		start.y + y,	start.z });

	glm::vec3 normals = glm::vec3(0);


	float t;
	glm::vec3 newVel = player.velocity;

	std::vector<std::pair<float, glm::vec3>> distances;
	if (sweptAABB(player, leftZ, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, rightZ, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, backX, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, frontX, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, leftFZ, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, rightFZ, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, backFX, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, frontFX, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, frontLY, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, frontRY, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, backLY, normals, t)) { distances.push_back({ t, normals }); }
	if (sweptAABB(player, backRY, normals, t)) { distances.push_back({ t, normals }); }

	std::sort(
		distances.begin(),
		distances.end(),
		[](const std::pair<float, glm::vec3> a, const std::pair<float, glm::vec3> b) {
			return a.first < b.first;
		});


	for (auto& pair : distances)
	{

		glm::vec3 absoluteVelocity = {
			std::abs(player.velocity.x),
			std::abs(player.velocity.y),
			std::abs(player.velocity.z)
		};

		player.velocity += pair.second * absoluteVelocity * (1 - pair.first);
	}

}
