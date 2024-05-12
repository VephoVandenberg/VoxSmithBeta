#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include <array>
#include <thread>
#include <mutex>
#include <algorithm>
#include <functional>
#include <future>
#include <string>
#ifdef _DEBUG
#include <iostream>
#endif

#include "../../engine/ray/ray.h"
#include "../../engine/shader/shader.h"
#include "../../engine/renderer/block_renderer.h"
#include "../../engine/renderer/mesh.h"
#include "../../engine/window/window.h"

#include "../chunk/chunk.h"
#include "../player/player.h"

#include "world.h"

using namespace GameModule;

constexpr glm::ivec3 g_chunkSize = { 16, 256, 16 };
constexpr size_t g_nBlocks = g_chunkSize.x * g_chunkSize.y * g_chunkSize.z;
constexpr size_t g_updateDistance = g_chunkSize.x * (g_chunksX / 2 - 1);

constexpr size_t g_width = 1280;
constexpr size_t g_height = 720;

std::mutex g_worldMutex;
std::vector<std::future<void>> g_futures;

void initParallelChunks(World& world, const glm::vec3& min, const glm::vec3& max)
{
	for (int32_t z = min.z; z < max.z; z += g_chunkSize.z)
	{
		for (int32_t x = min.x; x < max.x; x += g_chunkSize.x)
		{
			glm::ivec3 chunkPos = { x, 0, z };
			Chunk chunk = generateChunk(chunkPos);
			initChunkFaces(chunk);
			chunk.updated = false;

			std::lock_guard<std::mutex> lock(g_worldMutex);
			world.chunks[chunkPos] = std::move(chunk);
		}
	}
}

void initCascadeShadows(World& world, const Player& player)
{
	world.shadowCascadeLevels = {
		player.camera.farPlane / 20.0f,
		player.camera.farPlane / 5.0f
	};
}

void GameModule::initWorld(World& world, const Player& player)
{
	initCascadeShadows(world, player);
	Engine::initFArrayBuffer(world.shadowBuffer, world.shadowCascadeLevels);
	Engine::Renderer::initUBufferLM(world.lightSpaceMatricesUBO);

	world.pos = glm::ivec3(0);
	world.fractionPos = glm::vec3(0.0f);

	uint32_t maxThreads = std::thread::hardware_concurrency();
	uint32_t availableThreads = maxThreads - 1;
	world.threadsAvailable = availableThreads;
	std::vector<std::thread> threads;

	const float step = g_chunkSize.z * g_chunksZ / 2;

	int32_t minX, minZ;

	for (minZ = world.pos.z;
		minZ <= g_chunksZ * g_chunkSize.z - step;
		minZ += step)
	{
		for (minX = world.pos.x;
			minX <= g_chunksX * g_chunkSize.x - step;
			minX += step)
		{
			if (threads.size() >= availableThreads)
			{
				initParallelChunks(world,
					glm::vec3(minX, 0, minZ),
					glm::vec3(g_chunksX * g_chunkSize.x, 0.0f, g_chunksZ * g_chunkSize.z));
			}
			else
			{
				threads.push_back(
					std::move(
						std::thread(initParallelChunks,
							std::ref(world), glm::vec3(minX, 0, minZ), glm::vec3(minX + step, 0, minZ + step))));
			}
		}
	}
	for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

	for (int32_t z = 0; z < g_chunkSize.z * g_chunksZ; z += g_chunkSize.z)
	{
		for (int32_t x = 0; x < g_chunkSize.x * g_chunksX; x += g_chunkSize.x)
		{
			glm::ivec3 chunkPos = { x, 0, z };

			const glm::ivec3& left = world.chunks[chunkPos].left;
			if (world.chunks.find(left) != world.chunks.end())
			{
				updateChunkNeighbourFace(world.chunks[chunkPos], world.chunks[left]);
			}

			const glm::ivec3& back = world.chunks[chunkPos].back;
			if (world.chunks.find(back) != world.chunks.end())
			{
				updateChunkNeighbourFace(world.chunks[chunkPos], world.chunks[back]);
			}
		}
	}

	for (int32_t z = 0; z < g_chunksZ; z++)
	{
		for (int32_t x = 0; x < g_chunksX; x++)
		{
			glm::ivec3 chunkPos = { x * g_chunkSize.x, 0, z * g_chunkSize.z };


			for (auto& buffer : world.pool.buffers)
			{
				if (!buffer.active)
				{
					if (!world.chunks[chunkPos].solidBuffer)
					{
						world.chunks[chunkPos].solidBuffer = &buffer;
						buffer.active = true;
						world.pool.activeCounter++;
					}
					else if (!world.chunks[chunkPos].transBuffer)
					{
						world.chunks[chunkPos].transBuffer = &buffer;
						buffer.active = true;
						world.pool.activeCounter++;
					}
					else
					{
						break;
					}
					loadChunkMesh(world.chunks[chunkPos]);
					world.chunks[chunkPos].updated = true;
				}
			}
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

inline uint32_t getAbsId(const glm::ivec3& pos)
{
	return
		g_chunkSize.x * (g_chunkSize.z * (pos.y % g_chunkSize.y) + pos.z % g_chunkSize.z) + pos.x % g_chunkSize.x;
}

#define GET_ABS_ID(pos, size) size.x * (size.z * (pos.y % size.y) + pos.z % size.z) + pos.x % size.x

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

void GameModule::initChunkFaces(Chunk& chunk)
{
	for (uint32_t y = 0; y < g_chunkSize.y; y++)
	{
		for (uint32_t z = 0; z < g_chunkSize.z; z++)
		{
			for (uint32_t x = 0; x < g_chunkSize.x; x++)
			{
				glm::vec3 pos = glm::vec3(x, y, z);

				glm::vec3 top = { pos.x, pos.y + 1, pos.z };
				glm::vec3 front = { pos.x, pos.y, pos.z + 1 };
				glm::vec3 right = { pos.x + 1, pos.y, pos.z };

				uint32_t iBlock = g_chunkSize.x * (y * g_chunkSize.z + z) + x;

				uint32_t topId = g_chunkSize.x * (top.y * g_chunkSize.z + top.z) + top.x;
				uint32_t rightId = g_chunkSize.x * (right.y * g_chunkSize.z + right.z) + right.x;
				uint32_t frontId = g_chunkSize.x * (front.y * g_chunkSize.z + front.z) + front.x;

				if (chunk.blocks[iBlock].type == BlockType::AIR)
				{
					if (top.y < g_chunkSize.y &&
						chunk.blocks[topId].type != BlockType::AIR)
					{
						setBlockFace(chunk, top, chunk.blocks[topId].type, Face::FaceType::BOTTOM);
					}

					if (front.z < g_chunkSize.z &&
						chunk.blocks[frontId].type != BlockType::AIR)
					{
						setBlockFace(chunk, front, chunk.blocks[frontId].type, Face::FaceType::BACK);
					}

					if (right.x < g_chunkSize.x &&
						chunk.blocks[rightId].type != BlockType::AIR)
					{
						setBlockFace(chunk, right, chunk.blocks[rightId].type, Face::FaceType::LEFT);
					}
				}
				else if (chunk.blocks[iBlock].type == BlockType::WATER)
				{
					if (top.y < g_chunkSize.y &&
						(chunk.blocks[topId].type != BlockType::AIR &&
							chunk.blocks[topId].type != BlockType::WATER))
					{
						setBlockFace(chunk, top, chunk.blocks[topId].type, Face::FaceType::BOTTOM);
					}

					if (front.z < g_chunkSize.z &&
						(chunk.blocks[frontId].type != BlockType::AIR &&
							chunk.blocks[frontId].type != BlockType::WATER))
					{
						setBlockFace(chunk, front, chunk.blocks[frontId].type, Face::FaceType::BACK);
					}

					if (right.x < g_chunkSize.x &&
						(chunk.blocks[rightId].type != BlockType::AIR &&
							chunk.blocks[rightId].type != BlockType::WATER))
					{
						setBlockFace(chunk, right, chunk.blocks[rightId].type, Face::FaceType::LEFT);
					}

					if (top.y < g_chunkSize.y &&
						chunk.blocks[topId].type == BlockType::AIR)
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::TOP);
					}

					if (front.z < g_chunkSize.z &&
						chunk.blocks[frontId].type == BlockType::AIR)
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::FRONT);
					}

					if (right.x < g_chunkSize.x &&
						chunk.blocks[rightId].type == BlockType::AIR)
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::RIGHT);
					}
				}
				else
				{
					if (top.y < g_chunkSize.y &&
						(chunk.blocks[topId].type == BlockType::AIR ||
							chunk.blocks[topId].type == BlockType::WATER))
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::TOP);
					}

					if (front.z < g_chunkSize.z &&
						(chunk.blocks[frontId].type == BlockType::AIR ||
							chunk.blocks[frontId].type == BlockType::WATER))
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::FRONT);
					}

					if (right.x < g_chunkSize.x &&
						(chunk.blocks[rightId].type == BlockType::AIR ||
							chunk.blocks[rightId].type == BlockType::WATER))
					{
						setBlockFace(chunk, pos, chunk.blocks[iBlock].type, Face::FaceType::RIGHT);
					}
				}
			}
		}
	}
}

bool isChunkInTerrain(const World& world, const glm::ivec3& pos)
{
	return
		pos.x >= world.pos.x && pos.x < world.pos.x + g_chunkSize.x * g_chunksX &&
		pos.z >= world.pos.z && pos.z < world.pos.z + g_chunkSize.z * g_chunksZ;
}

void addChunks(World& world)
{
	//while (!world.chunksToAdd.empty())
	{
		Chunk chunk = generateChunk(*world.chunksToAdd.begin());
		initChunkFaces(chunk);

		if (world.chunks.find(chunk.front) != world.chunks.end())
		{
			updateChunkNeighbourFace(chunk, world.chunks[chunk.front]);
		}

		if (world.chunks.find(chunk.back) != world.chunks.end())
		{
			updateChunkNeighbourFace(chunk, world.chunks[chunk.back]);
		}

		if (world.chunks.find(chunk.right) != world.chunks.end())
		{
			updateChunkNeighbourFace(chunk, world.chunks[chunk.right]);
		}

		if (world.chunks.find(chunk.left) != world.chunks.end())
		{
			updateChunkNeighbourFace(chunk, world.chunks[chunk.left]);
		}

		chunk.updated = false;
		world.chunks.insert({ *world.chunksToAdd.begin(), chunk });

		for (auto& buffer : world.pool.buffers)
		{
			if (!buffer.active)
			{
				if (!world.chunks[*world.chunksToAdd.begin()].solidBuffer)
				{
					world.chunks[*world.chunksToAdd.begin()].solidBuffer = &buffer;
					buffer.active = true;
					world.pool.activeCounter++;
				}
				else if (!world.chunks[*world.chunksToAdd.begin()].transBuffer)
				{
					world.chunks[*world.chunksToAdd.begin()].transBuffer = &buffer;
					buffer.active = true;
					world.pool.activeCounter++;
				}
				else
				{
					break;
				}
			}

			if (world.pool.activeCounter == world.pool.buffers.size())
			{
				break;
			}
		}

		world.chunksToAdd.erase(world.chunksToAdd.begin());
	}
}

void GameModule::updateWorld(World& world, const Player& player, float dt)
{
	// glm::mat4 model = glm::mat4(1.0f);
	// model = glm::rotate(model, glm::radians(dt) * 10, glm::vec3(0.0f, 0.0f, 1.0f));
	// world.lightDir = glm::normalize(glm::vec3(model * glm::vec4(world.lightDir, 1.0f)));

	return;

	for (int32_t z = world.pos.z;
		z < world.pos.z + g_chunksZ * g_chunkSize.z;
		z += g_chunkSize.z)
	{
		for (int32_t x = world.pos.x;
			x < world.pos.x + g_chunksX * g_chunkSize.x;
			x += g_chunkSize.x)
		{
			if (world.chunks.find({ x, 0, z }) == world.chunks.end() &&
				world.chunksToRemove.count({ x, 0, z }) == 0)
			{
				world.chunksToAdd.insert({ x, 0, z });
			}
		}
	}

	for (const auto& pair : world.chunks)
	{
		if (!isChunkInTerrain(world, pair.first))
		{
			world.chunksToRemove.insert(pair.first);
		}
	}

	if (!world.chunksToRemove.empty())
	{
		world.pool.activeCounter -= disableChunk(world.chunks[*world.chunksToRemove.begin()]);
		world.chunks.erase(*world.chunksToRemove.begin());
		world.chunksToRemove.erase(world.chunksToRemove.begin());
	}

	if (!world.chunksToAdd.empty() && world.pool.buffers.size() - world.pool.activeCounter >= 2)
	{
		addChunks(world);
	}
}

glm::mat4 getLightSpaceMatrix(const World& world, const Player& player, 
	const float nearPlane, const float farPlane,
	const float prevSplitD, const float currSplitD)
{
	std::vector<glm::vec3> boundingVertices = {
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f,  1.0f, -1.0f),
		glm::vec3( 1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3( 1.0f,  1.0f,  1.0f),
		glm::vec3( 1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
	};

	const auto eyeViewMatrix = player.camera.view;
	const auto projectionMatrix = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(g_width) / static_cast<float>(g_height),
		nearPlane, farPlane);
	const auto inv = glm::inverse(projectionMatrix * eyeViewMatrix);

	for (unsigned int i = 0; i < boundingVertices.size(); i++)
	{
		glm::vec4 invPoint = inv * glm::vec4(boundingVertices[i], 1.0f);
		boundingVertices[i] = glm::vec3(invPoint / invPoint.w);
	}

	for (unsigned int i = 0; i < boundingVertices.size() / 2.0f; i++)
	{
		glm::vec3 cornerRay = boundingVertices[i + 4] - boundingVertices[i];
		glm::vec3 nearCornerRay = cornerRay * prevSplitD;
		glm::vec3 farCornerRay = cornerRay * currSplitD;
		boundingVertices[i + 4] = boundingVertices[i] + farCornerRay;
		boundingVertices[i] = boundingVertices[i] + nearCornerRay;
	}

	glm::vec3 center = glm::vec3(0.0f);
	for (unsigned int i = 0; i < boundingVertices.size(); i++)
	{
		center += boundingVertices[i];
	}
	center /= 8.0f;

	float radius = 0.0f;
	for (unsigned int i = 0; i < boundingVertices.size(); i++)
	{
		float distance = glm::length(boundingVertices[i] - center);
		radius = std::max(radius, distance);
	}

	radius = std::ceil(radius * 16.0f) / 16.0f;

	glm::vec3 maxExtents = glm::vec3(radius);
	glm::vec3 minExtents = -maxExtents;

	// Position the viewmatrix looking down the center of the frustum with an arbitrary lighht direction
	glm::vec3 lightDirection = center - glm::normalize(-world.lightDir) * - minExtents.z;
	glm::mat4 lightViewMatrix = glm::mat4(1.0f);
	lightViewMatrix = glm::lookAt(lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 cascadeExtents = maxExtents - minExtents;

	glm::mat4 lightOrthoMatrix = 
		glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, cascadeExtents.z);

	glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
	glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	shadowOrigin = shadowMatrix * shadowOrigin;
	shadowOrigin = shadowOrigin * Engine::g_shadowResolution / 2.0f;

	glm::vec4 roundedOrigin = glm::round(shadowOrigin);
	glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
	roundOffset = roundOffset * 2.0f / Engine::g_shadowResolution;
	roundOffset.z = 0.0f;
	roundOffset.w = 0.0f;

	glm::mat4 shadowProj = lightOrthoMatrix;
	shadowProj[3] += roundOffset;
	lightOrthoMatrix = shadowProj;

	return shadowProj * lightViewMatrix;
}

std::vector<glm::mat4> getLightSpaceMatrices(const World& world, const Player& player)
{
	std::vector<glm::mat4> ret;
	float prevSplitDistance = 0.1f;
	float currSplitDistance;
	for (size_t i = 0; i < world.shadowCascadeLevels.size() + 1; i++)
	{
		if (i == 0)
		{
			currSplitDistance = world.shadowCascadeLevels[i] - player.camera.nearPlane;
			ret.push_back(getLightSpaceMatrix(world, player, 
				player.camera.nearPlane, world.shadowCascadeLevels[i], prevSplitDistance, currSplitDistance));
		}
		else if (i < world.shadowCascadeLevels.size())
		{
			currSplitDistance = world.shadowCascadeLevels[i] - world.shadowCascadeLevels[i - 1];
			ret.push_back(getLightSpaceMatrix(world, player, 
				world.shadowCascadeLevels[i - 1], world.shadowCascadeLevels[i], prevSplitDistance, currSplitDistance));
		}
		else
		{
			currSplitDistance = player.camera.farPlane - world.shadowCascadeLevels[i - 1];
			ret.push_back(getLightSpaceMatrix(world, player,
				world.shadowCascadeLevels[i - 1], player.camera.farPlane, prevSplitDistance, currSplitDistance));
		}

		prevSplitDistance = currSplitDistance;
	}
	return ret;
}

void GameModule::drawWorlToSM(World& world, Player& player, Engine::Shader& shader)
{
	Engine::Renderer::updateUBufferLM(world.lightSpaceMatricesUBO, getLightSpaceMatrices(world, player));

	Engine::bindFBuffer(world.shadowBuffer);
	Engine::setFramebufferViewport();
	Engine::clearDepthBuff();
	Engine::Renderer::disableCulling();

	for (auto& pair : world.chunks)
	{
		if (!pair.second.updated)
		{
			loadChunkMesh(pair.second);
			pair.second.updated = true;
		}
		Engine::setUniform3f(shader, "u_chunkPos", pair.second.pos);
		drawSolid(pair.second);
	}
	Engine::Renderer::enableCulling();
	Engine::unbindFBuffer();
}

#ifdef _DEBUG
void GameModule::drawDebugQuad(World& world, Engine::Shader& shader)
{
	Engine::useFArray(world.shadowBuffer);
	Engine::Renderer::render(Engine::Renderer::Type::QUAD);
}
#endif

void GameModule::drawWorld(World& world, const Player& player, Engine::Shader& shader)
{
	Engine::useFArray(world.shadowBuffer);
	std::lock_guard<std::mutex> lock(g_worldMutex);
	std::multimap<float, glm::ivec3> sorted;

	for (uint32_t i = 0; i < world.shadowCascadeLevels.size(); i++)
	{
		const std::string uniform = "u_cascadePlaneDistances[" + std::to_string(i) + "]";
		Engine::setUniformf(shader, uniform.c_str(), world.shadowCascadeLevels[i]);
	}

	Engine::setUniform3f(shader, "u_lightDir", world.lightDir);
	Engine::setUniformf(shader, "u_farPlane", player.camera.farPlane);
	Engine::setUniformi(shader, "u_cascadeCount", world.shadowCascadeLevels.size());
	for (auto& pair : world.chunks)
	{
		if (!pair.second.updated)
		{
			loadChunkMesh(pair.second);
			pair.second.updated = true;
		}
		Engine::setUniform3f(shader, "u_chunkPos", pair.second.pos);
		drawSolid(pair.second);

		if (!pair.second.transparentMesh.empty())
		{
			float distance = glm::length(player.camera.pos - static_cast<const glm::vec3>(pair.first));
			sorted.insert({ distance, pair.first });
		}
	}


	Engine::Renderer::disableCulling();
	for (auto it = sorted.rbegin(); it != sorted.rend(); it++)
	{
		Engine::setUniform3f(shader, "u_chunkPos", it->second);
		drawTrans(world.chunks[it->second]);
	}
	Engine::Renderer::enableCulling();
}

bool collAABB(const Player& player, const glm::vec3& pos)
{
	bool collX = player.pos.x + player.size.x > pos.x && pos.x + 1 > player.pos.x;
	bool collZ = player.pos.z + player.size.z > pos.z && pos.z + 1 > player.pos.z;
	bool collY = player.pos.y + player.size.y > pos.y && pos.y + 1 > player.pos.y;
	return collX && collY && collZ;
}

void GameModule::processRay(World& world, const Player& player,
	Engine::Ray& ray, Engine::Shader& shader, RayType type)
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

	glm::ivec3 rightBlock = { currBlock.x + 1, currBlock.y, currBlock.z };
	glm::ivec3 leftBlock = { currBlock.x - 1, currBlock.y, currBlock.z };
	glm::ivec3 frontBlock = { currBlock.x, currBlock.y, currBlock.z + 1 };
	glm::ivec3 backBlock = { currBlock.x, currBlock.y, currBlock.z - 1 };

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

	const Block& leftZ = getBlock(world, { start.x,	start.y, start.z + z });
	const Block& rightZ = getBlock(world, { start.x + 1.0f,	start.y, start.z + z });
	const Block& backX = getBlock(world, { start.x + x,	start.y, start.z });
	const Block& frontX = getBlock(world, { start.x + x, start.y, start.z + 1.0f });
	const Block& leftFZ = getBlock(world, { start.x, start.y + 1.0f, start.z + z });
	const Block& rightFZ = getBlock(world, { start.x + 1.0f, start.y + 1.0f, start.z + z });
	const Block& backFX = getBlock(world, { start.x + x, start.y + 1.0f, start.z });
	const Block& frontFX = getBlock(world, { start.x + x, start.y + 1.0f, start.z + 1.0f });
	const Block& frontLY = getBlock(world, { start.x, start.y + y, start.z + 1.0f });
	const Block& frontRY = getBlock(world, { start.x + 1.0f, start.y + y, start.z + 1.0f });
	const Block& backLY = getBlock(world, { start.x, start.y + y, start.z });
	const Block& backRY = getBlock(world, { start.x + 1.0f,	start.y + y, start.z });

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
