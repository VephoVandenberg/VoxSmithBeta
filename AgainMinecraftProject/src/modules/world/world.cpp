#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <array>
#include <thread>
#include <glfw3.h>

#include "../../engine/ray/ray.h"
#include "../../engine/shader/shader.h"
#include "../../engine/renderer/block_renderer.h"

#include "../chunk/chunk.h"
#include "../player/player.h"

#include "world.h"

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };

constexpr int32_t g_chunksX = 32;
constexpr int32_t g_chunksZ = 32;

constexpr size_t g_nBlocks = g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;

void GameModule::initWorld(World& world)
{
	world.minBorder = glm::ivec3(0);
	world.maxBorder = glm::ivec3(g_chunkSize.x * g_chunksX, 0, g_chunkSize.z * g_chunksZ);

	float t = glfwGetTime();
	
	std::vector<std::thread> threads(std::thread::hardware_concurrency() - 1);

	for (int32_t z = 0; z < world.maxBorder.z; z += g_chunkSize.z)
	{
		for (int32_t x = 0; x < world.maxBorder.x; x += g_chunkSize.x)
		{
			glm::ivec3 chunkPos = { x, 0, z };

			world.chunks[chunkPos] = generateChunk(chunkPos);
			initChunkFaces(world, world.chunks[chunkPos]);

			glm::ivec3 pos = { x - g_chunkSize.x, 0, z };
			if (world.chunks.find(pos) != world.chunks.end())
			{
				updateChunkNeighbourFace(world.chunks[chunkPos], world.chunks[pos]);
			}

			pos = { x, 0, z - g_chunkSize.z };
			if (world.chunks.find(pos) != world.chunks.end())
			{
				updateChunkNeighbourFace(world.chunks[chunkPos], world.chunks[pos]);
			}
		}
	}
	std::cout << glfwGetTime() - t << std::endl;

	for (int32_t z = 0; z < g_chunksZ; z++)
	{
		for (int32_t x = 0; x < g_chunksX; x++)
		{
			glm::ivec3 chunkPos = { x * g_chunkSize.x, 0, z * g_chunkSize.z };
			
			loadChunkMesh(world.chunks[chunkPos]);
		}
	}
}

inline glm::ivec3 getChunkPos(const glm::vec3 pos)
{
	return {
		static_cast<int32_t>(pos.x) / g_chunkSize.x * g_chunkSize.x,
		static_cast<int32_t>(pos.y) / g_chunkSize.y * g_chunkSize.y,
		static_cast<int32_t>(pos.z) / g_chunkSize.z * g_chunkSize.z
	};
}

inline uint32_t getId(World& world, const glm::ivec3 pos)
{
	glm::ivec3 iPos = pos - getChunkPos(pos);

	return g_chunkSize.x * (g_chunkSize.z * (iPos.y) + iPos.z) + iPos.x;
}

inline uint32_t getAbsId(glm::ivec3 pos)
{
	return 
		g_chunkSize.x * (g_chunkSize.z * (pos.y % g_chunkSize.y) + pos.z % g_chunkSize.z) + pos.x % g_chunkSize.x;
}

Block& getBlock(World& world, const glm::vec3 pos)
{
	glm::ivec3 p = static_cast<glm::ivec3>(pos);

	return world.chunks[getChunkPos(pos)].blocks[getId(world, pos)];
}


inline bool isEdge(World& world, const glm::vec3 pos)
{
	return world.chunks.find(getChunkPos(pos)) == world.chunks.end();
}


bool isBlockSolid(World& world, const glm::vec3 pos)
{
	glm::vec3 chunkPos = getChunkPos(pos);
	if (world.chunks.find(chunkPos) == world.chunks.end())
	{
		return false;
	}

	return getBlock(world, pos).type != BlockType::AIR;
}

bool isBlockTrans(World& world, const glm::vec3 pos)
{
	glm::vec3 chunkPos = getChunkPos(pos);
	if (world.chunks.find(chunkPos) == world.chunks.end())
	{
		return false;
	}

	return getBlock(world, pos).type == BlockType::AIR;
}

void GameModule::initChunkFaces(World& world, Chunk& chunk)
{
	for (uint32_t y = 0; y < g_chunkSize.y; y++)
	{
		for (uint32_t z = 0; z < g_chunkSize.z; z++)
		{
			for (uint32_t x = 0; x < g_chunkSize.x; x++)
			{
				glm::vec3 pos = chunk.pos + glm::vec3( x, y, z );

				glm::vec3 top = { pos.x, pos.y + 1, pos.z };
				glm::vec3 front = { pos.x, pos.y, pos.z + 1 };
				glm::vec3 right = { pos.x + 1, pos.y, pos.z };

				uint32_t iBlock = getAbsId(pos);

				uint32_t topId = getAbsId(top);
				uint32_t rightId = getAbsId(right);
				uint32_t frontId = getAbsId(front);

				if (chunk.blocks[getAbsId(pos)].type == BlockType::AIR)
				{
					bool topSolid = top.y < g_chunkSize.y && chunk.blocks[topId].type != BlockType::AIR;
					bool frontSolid = front.z < chunk.pos.z + g_chunkSize.z && chunk.blocks[frontId].type != BlockType::AIR;
					bool rightSolid = right.x < chunk.pos.x + g_chunkSize.x && chunk.blocks[rightId].type != BlockType::AIR;

					if (topSolid)
					{
						setBlockFace(chunk, top, Face::FaceType::BOTTOM);
					}

					if (frontSolid)
					{
						setBlockFace(chunk, front, Face::FaceType::BACK);
					}

					if (rightSolid)
					{
						setBlockFace(chunk, right, Face::FaceType::LEFT);
					}
				}
				else
				{
					bool topTrans = top.y < g_chunkSize.y && chunk.blocks[topId].type == BlockType::AIR;
					bool frontTrans = front.z < chunk.pos.z + g_chunkSize.z && chunk.blocks[frontId].type == BlockType::AIR;
					bool rightTrans = right.x < chunk.pos.x + g_chunkSize.x && chunk.blocks[rightId].type == BlockType::AIR;

					if (topTrans)
					{
						setBlockFace(chunk, pos, Face::FaceType::TOP);
					}

					if (frontTrans)
					{
						setBlockFace(chunk, pos, Face::FaceType::FRONT);
					}

					if (rightTrans)
					{
						setBlockFace(chunk, pos, Face::FaceType::RIGHT);
					}
				}
			}
		}
	}
}

void GameModule::drawWorld(World& world)
{
	for (auto& pair : world.chunks)
	{
		if (!pair.second.updated)
		{
			loadChunkMesh(pair.second);
			pair.second.updated = true;
		}
		drawChunk(pair.second);
	}
}

bool collAABB(const Player& player, const glm::vec3& pos)
{
	bool collX = player.pos.x + player.size.x > pos.x && pos.x + 1 > player.pos.x;
	bool collZ = player.pos.z + player.size.z > pos.z && pos.z + 1 > player.pos.z;
	bool collY = player.pos.y + player.size.y > pos.y && pos.y + 1 > player.pos.y;
	return collX && collY && collZ;
}

void GameModule::updateWorld(World& world, Player& player)
{

}

void GameModule::processRay(World& world, const Player& player, Engine::Ray& ray, Engine::Shader& shader, RayType type)
{
	bool		hit = false;
	float		dl = 0.1f;
	float		length = glm::length(ray.end - ray.start);

	glm::vec3	dir = glm::normalize(ray.end - ray.start);
	glm::vec3	currPos = {};

	glm::ivec3	iPos = glm::ivec3(0);
	glm::ivec3	currChunkPos = {};

	for (float mag = 0.0f; mag < length; mag += dl)
	{
		currPos = ray.start + mag * dir;
		currChunkPos = getChunkPos(currPos);

		if (world.chunks.find(currChunkPos) == world.chunks.end())
		{
			continue;
		}

		iPos = static_cast<glm::ivec3>(currPos) - currChunkPos;
		uint32_t id = g_chunkSize.x * (g_chunkSize.x * iPos.y + iPos.z) + iPos.x;

		if (world.chunks[currChunkPos].blocks[id].type != BlockType::AIR)
		{
			if (type == RayType::PLACE)
			{
				currPos -= dl * dir;
				currChunkPos = getChunkPos(currPos);
				iPos = static_cast<glm::ivec3>(currPos) - currChunkPos;
				currPos -= currChunkPos;
				id = g_chunkSize.x * (g_chunkSize.x * iPos.y + iPos.z) + iPos.x;
				if (collAABB(player, currPos))
				{
					break;
				}
			}
			
			traceRay(world, currPos, shader, type);
			hit = true;
			break;
		}
	}
}

void GameModule::traceRay(World& world, glm::vec3 rayPosFrac, Engine::Shader& shader, GameModule::RayType type)
{
	glm::ivec3 currBlock = static_cast<glm::ivec3>(rayPosFrac);

	glm::ivec3 rightBlock = { currBlock.x + 1,	currBlock.y,		currBlock.z };
	glm::ivec3 leftBlock = { currBlock.x - 1,	currBlock.y,		currBlock.z };
	glm::ivec3 frontBlock = { currBlock.x,		currBlock.y,		currBlock.z + 1 };
	glm::ivec3 backBlock = { currBlock.x,		currBlock.y,		currBlock.z - 1 };

	glm::ivec3 iPosCurr = currBlock - getChunkPos(currBlock);
	glm::ivec3 iPosRight = rightBlock - getChunkPos(rightBlock);
	glm::ivec3 iPosLeft = leftBlock - getChunkPos(leftBlock);
	glm::ivec3 iPosFront = frontBlock - getChunkPos(frontBlock);
	glm::ivec3 iPosBack = backBlock - getChunkPos(backBlock);

	uint32_t iCurr = g_chunkSize.x * (g_chunkSize.z * iPosCurr.y + iPosCurr.z) + iPosCurr.x;
	uint32_t iTop = g_chunkSize.x * (g_chunkSize.z * (iPosCurr.y + 1) + iPosCurr.z) + iPosCurr.x;
	uint32_t iBottom = g_chunkSize.x * (g_chunkSize.z * (iPosCurr.y - 1) + iPosCurr.z) + iPosCurr.x;

	uint32_t iRight = g_chunkSize.x * (g_chunkSize.z * iPosRight.y + iPosRight.z) + iPosRight.x;
	uint32_t iLeft = g_chunkSize.x * (g_chunkSize.z * iPosLeft.y + iPosLeft.z) + iPosLeft.x;
	uint32_t iFront = g_chunkSize.x * (g_chunkSize.z * iPosFront.y + iPosFront.z) + iPosFront.x;
	uint32_t iBack = g_chunkSize.x * (g_chunkSize.z * iPosBack.y + iPosBack.z) + iPosBack.x;

	bool rightSolid = world.chunks[getChunkPos(rightBlock)].blocks[iRight].type != BlockType::AIR;
	bool leftSolid = world.chunks[getChunkPos(leftBlock)].blocks[iLeft].type != BlockType::AIR;
	bool topSolid = world.chunks[getChunkPos(currBlock)].blocks[iTop].type != BlockType::AIR;
	bool bottomSolid = world.chunks[getChunkPos(currBlock)].blocks[iBottom].type != BlockType::AIR;
	bool frontSolid = world.chunks[getChunkPos(frontBlock)].blocks[iFront].type != BlockType::AIR;
	bool backSolid = world.chunks[getChunkPos(backBlock)].blocks[iBack].type != BlockType::AIR;
	/*
	if (type == RayType::REMOVE)
	{
		auto& curr = world.chunks[getChunkPos(currBlock)];
		world.chunks[getChunkPos(currBlock)].blocks[iCurr].type = BlockType::AIR;

		//curr.faces.erase(
		//	std::remove_if(
		//		curr.faces.begin(), curr.faces.end(),
		//		[&](const Face& face) {
		//			return face.blockID == iCurr;
		//		}),
		//	curr.faces.end()
		//			);
		curr.updated = false;

		if (rightSolid) { setBlockFace(world.chunks[getChunkPos(rightBlock)], iRight, Face::FaceType::LEFT); }
		if (leftSolid) { setBlockFace(world.chunks[getChunkPos(leftBlock)], iLeft, Face::FaceType::RIGHT); }
		if (topSolid) { setBlockFace(world.chunks[getChunkPos(currBlock)], iTop, Face::FaceType::BOTTOM); }
		if (bottomSolid) { setBlockFace(world.chunks[getChunkPos(currBlock)], iBottom, Face::FaceType::TOP); }
		if (frontSolid) { setBlockFace(world.chunks[getChunkPos(frontBlock)], iFront, Face::FaceType::BACK); }
		if (backSolid) { setBlockFace(world.chunks[getChunkPos(backBlock)], iBack, Face::FaceType::FRONT); }
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
	*/
}

bool sweptAABB(const Player& player, const Block& block, glm::vec3& normals, float dt, float& hitTime)
{
	float dxEntry, dzEntry, dyEntry;
	float dxExit, dzExit, dyExit;

	glm::vec3 vel = glm::normalize(player.velocity) * dt;
	/*
	if (player.velocity.x > 0.0f)
	{
		dxEntry = block.pos.x - (player.pos.x + player.size.x);
		dxExit = block.pos.x + 1.0f - player.pos.x;
	}
	else
	{
		dxEntry = block.pos.x + 1.0f - player.pos.x;
		dxExit = block.pos.x - (player.pos.x + player.size.x);
	}

	if (player.velocity.z > 0.0f)
	{
		dzEntry = block.pos.z - (player.pos.z + player.size.z);
		dzExit = block.pos.z + 1.0f - player.pos.z;
	}
	else
	{
		dzEntry = block.pos.z + 1.0f - player.pos.z;
		dzExit = block.pos.z - (player.pos.z + player.size.z);
	}

	if (player.velocity.y > 0.0f)
	{
		dyEntry = block.pos.y - (player.pos.y + player.size.y);
		dyExit = block.pos.y + 1.0f - player.pos.y;
	}
	else
	{
		dyEntry = block.pos.y + 1.0f - player.pos.y;
		dyExit = block.pos.y - (player.pos.y + player.size.y);
	}

	float txEntry, tzEntry, tyEntry;
	float txExit, tzExit, tyExit;

	if (player.velocity.x == 0.0f)
	{
		txEntry = -std::numeric_limits<float>::infinity();
		txExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		txEntry = dxEntry / vel.x;
		txExit = dxExit / vel.x;
	}

	if (player.velocity.z == 0.0f)
	{
		tzEntry = -std::numeric_limits<float>::infinity();
		tzExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		tzEntry = dzEntry / vel.z;
		tzExit = dzExit / vel.z;
	}

	if (player.velocity.y == 0.0f)
	{
		tyEntry = -std::numeric_limits<float>::infinity();
		tyExit = std::numeric_limits<float>::infinity();
	}
	else
	{
		tyEntry = dyEntry / vel.y;
		tyExit = dyExit / vel.y;
	}

	hitTime = std::max(std::max(txEntry, tzEntry), tyEntry);
	float exitTime = std::min(std::min(txExit, tzExit), tyExit);

	if (block.type == BlockType::AIR ||
		hitTime > exitTime ||
		(txEntry < 0.0f && tzEntry < 0.0f && tyEntry < 0.0f)
		|| txEntry > 1.0f || tzEntry > 1.0f || tyEntry > 1.0f)
	{
		return false;
	}

	if (txEntry > tyEntry)
	{
		if (txEntry > tzEntry)
		{
			if (player.velocity.x < 0.0f)
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
			if (player.velocity.z < 0.0f)
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
		if (tyEntry > tzEntry)
		{
			if (player.velocity.y < 0.0f)
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
			if (player.velocity.z < 0.0f)
			{
				normals = { 0, 0, 1 };
			}
			else
			{
				normals = { 0, 0, -1 };
			}
		}
	}
	*/
	return true;
}

void GameModule::checkPlayerCollision(World& world, Player& player, float dt)
{
	float x = player.velocity.x > 0.0f ? 1 : -1;
	float z = player.velocity.z > 0.0f ? 1 : -1;
	float y = player.velocity.y > 0.0f ? 3 : -1;

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

	std::vector<std::pair<float, const Block*>> distances;
	if (sweptAABB(player, leftZ, normals, dt, t)) { distances.push_back({ t, &leftZ }); }
	if (sweptAABB(player, rightZ, normals, dt, t)) { distances.push_back({ t, &rightZ }); }
	if (sweptAABB(player, backX, normals, dt, t)) { distances.push_back({ t, &backX }); }
	if (sweptAABB(player, frontX, normals, dt, t)) { distances.push_back({ t, &frontX }); }
	if (sweptAABB(player, leftFZ, normals, dt, t)) { distances.push_back({ t, &leftFZ }); }
	if (sweptAABB(player, rightFZ, normals, dt, t)) { distances.push_back({ t, &rightFZ }); }
	if (sweptAABB(player, backFX, normals, dt, t)) { distances.push_back({ t, &backFX }); }
	if (sweptAABB(player, frontFX, normals, dt, t)) { distances.push_back({ t, &frontFX }); }
	if (sweptAABB(player, frontLY, normals, dt, t)) { distances.push_back({ t, &frontLY }); }
	if (sweptAABB(player, frontRY, normals, dt, t)) { distances.push_back({ t, &frontRY }); }
	if (sweptAABB(player, backLY, normals, dt, t)) { distances.push_back({ t, &backLY }); }
	if (sweptAABB(player, backRY, normals, dt, t)) { distances.push_back({ t, &backRY }); }

	std::sort(distances.begin(), distances.end(),
		[](const std::pair<float, const Block*>& a, const std::pair<float, const Block*>& b) {
			return a.first < b.first;
		});

	for (auto& pair : distances)
	{
		glm::vec3 absoluteVelocity = { std::abs(player.velocity.x), std::abs(player.velocity.y), std::abs(player.velocity.z) };
		glm::vec3 normals = glm::vec3(0);
		float t;
		if (sweptAABB(player, *pair.second, normals, dt, t))
		{
			player.velocity += normals * absoluteVelocity * (1 - t);
		}
	}
}

bool GameModule::isPlayerFalling(World& world, const Player& player, float dt)
{
	const glm::vec3 start = { player.pos.x, player.pos.y, player.pos.z };

	const Block& frontLY = getBlock(world, { start.x,				start.y - 1.0f,	start.z + 1.0f });
	const Block& frontRY = getBlock(world, { start.x + 1.0f,		start.y - 1.0f,	start.z + 1.0f });
	const Block& backLY = getBlock(world, { start.x,				start.y - 1.0f,	start.z });
	const Block& backRY = getBlock(world, { start.x + 1.0f,		start.y - 1.0f,	start.z });

	glm::vec3 normals;
	float t;

	return
		!sweptAABB(player, frontLY, normals, dt, t) && !sweptAABB(player, frontRY, normals, dt, t) &&
		!sweptAABB(player, backLY, normals, dt, t) && !sweptAABB(player, backRY, normals, dt, t);
}
