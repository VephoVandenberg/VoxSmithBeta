#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <mutex>
#include <thread>
#include <algorithm>

#include "../engine/window/window.h"
#include "../engine/shader/shader_list.h"

#include "../engine/ray/ray.h"
#include "../engine/renderer/block_renderer.h"

#ifdef ECS
#include "../modules/chunk/block.h"
#else
#include "../modules/chunk/chunk.h"
#endif

#include "app.h"

using namespace Engine;
#ifndef ECS
#endif
using namespace GameModule;
using namespace App;


const char* g_title = "Azamat's making Minecraft fucking again";
constexpr size_t g_width = 1240;
constexpr size_t g_height = 720;

constexpr size_t g_numberOfChunksX = 8;
constexpr size_t g_numberOfChunksZ = 8;

constexpr int32_t g_chunkOffsetX = 16;
constexpr int32_t g_chunkOffsetZ = 16;
constexpr int32_t g_chunkOffsetY = 256;

Ray m_ray;
Renderer::Buffer m_rayBuffer;

Application::Application()
	: m_isRunning(true)
{
	init();
}

Application::~Application()
{
	freeCamera();
}

void Application::init()
{
	m_window = getWindow(g_title, g_width, g_height);

	initCamera(m_window, glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(g_chunkOffsetX * g_numberOfChunksX / 2, g_chunkOffsetY / 2 + 1, g_chunkOffsetZ  * g_numberOfChunksZ / 2));
	initShaders();
	initTextures();

#ifdef ECS
#else
	initWorld(m_world);
#endif
}

void Application::initShaders()
{
	Engine::loadShaders(m_shaders);

	glm::mat4 projection =
		glm::perspective(
			glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);

	useShader(m_shaders[s_cubeShader]);
	setUniform4m(m_shaders[s_cubeShader], "u_projection",	projection);
	setUniform4m(m_shaders[s_cubeShader], "u_view",			getCameraView());

	useShader(m_shaders[s_meshShader]);
	setUniform4m(m_shaders[s_meshShader], "u_projection",	projection);
	setUniform4m(m_shaders[s_meshShader], "u_view",			getCameraView());

	useShader(m_shaders[s_rayShader]);
	setUniform4m(m_shaders[s_rayShader], "u_projection",	projection);
	setUniform4m(m_shaders[s_rayShader], "u_view",			getCameraView());
}

void Application::initTextures()
{
	Texture texture;
	initTexture(texture, "textures/grass.png");
	m_textures["grass"] = texture;
}

void Application::run()
{
	useTexture(m_textures["grass"]);
	while (m_isRunning)
	{
		onUpdate();
		clearScreen();

		onRender();

		handleInput();
		updateScreen(m_window);
	}
}

void Application::handleInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_W] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_W] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_A] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_A] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_D] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_D] = false;
	}

	if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_keyboard[GLFW_KEY_S] = true;
	}
	else
	{
		m_keyboard[GLFW_KEY_S] = false;
	}

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT])
	{
		m_keyboard[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		m_keyboard[GLFW_MOUSE_BUTTON_LEFT] = false;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = false;
	}

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS &&
		!m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] = true;
	}
	else if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] = false;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = false;
	}
}

void Application::onRender()
{
#ifdef ECS
#else
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());
	drawWorld(m_world);
#endif

	setUniform4m(m_shaders[s_rayShader], "u_view", getCameraView());
	Renderer::render(Renderer::Type::RAY);
}

void Application::onUpdate()
{
	updateCameraMove(m_keyboard);
	
	if (m_keyboard[GLFW_MOUSE_BUTTON_LEFT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] ||
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
		Ray ray = castRay();
		Renderer::loadRayData(ray);

#ifdef ECS
#else
		auto type = m_keyboard[GLFW_MOUSE_BUTTON_LEFT] ? RayType::REMOVE : RayType::PLACE;
		processRay(m_world, ray, type);
				
#endif
		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
}

/*
void Application::updateTerrain()
{
	constexpr size_t updateDistance = g_chunkOffsetX;

	if (true)
	{
		return;
	}
	if (glm::length(getCameraPos().x - m_minBorder.x) < updateDistance)
	{
		m_minBorder.x -= g_chunkOffsetX;
		for (int32_t z = m_minBorder.z; z < m_maxBorder.z; z += g_chunkOffsetZ)
		{
			glm::vec3 newPos = { m_minBorder.x, m_minBorder.y, z };
			glm::vec3 oldPos = { m_maxBorder.x, m_minBorder.y, z };

			m_chunks[newPos] = generateChunk(newPos);
			initMeshData(m_chunks[newPos]);
			generateMesh(m_chunks[newPos]);
			loadChunkMesh(m_chunks[newPos]);

			if (m_chunks.find(oldPos) != m_chunks.end())
			{
				m_chunks.erase(oldPos);
			}
		}

		m_maxBorder.x -= g_chunkOffsetX;

		//flag = true;
	}
	else if (glm::length(getCameraPos().x - m_maxBorder.x) < updateDistance)
	{
		
	}
	else if (glm::length(getCameraPos().z - m_minBorder.z) < updateDistance)
	{
 		
	}
	else if (glm::length(getCameraPos().z - m_maxBorder.z) < updateDistance)
	{
		
	}
}

void Application::traceRay(glm::vec3 rayPosFrac, RayType type)
{
	auto getChunkPos = [](const glm::vec3 pos) {
		return glm::ivec3(
			static_cast<int32_t>(pos.x) / g_chunkOffsetX * g_chunkOffsetX,
			0,
			static_cast<int32_t>(pos.z) / g_chunkOffsetZ * g_chunkOffsetZ
		);
	};

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

	uint32_t iCurr = g_chunkOffsetX * (g_chunkOffsetZ * iPosCurr.y + iPosCurr.z) + iPosCurr.x;
	uint32_t iTop = g_chunkOffsetX * (g_chunkOffsetZ * (iPosCurr.y + 1) + iPosCurr.z) + iPosCurr.x;
	uint32_t iBottom = g_chunkOffsetX * (g_chunkOffsetZ * (iPosCurr.y - 1) + iPosCurr.z) + iPosCurr.x;

	uint32_t iRight = g_chunkOffsetX * (g_chunkOffsetZ * iPosRight.y + iPosRight.z) + iPosRight.x;
	uint32_t iLeft = g_chunkOffsetX * (g_chunkOffsetZ * iPosLeft.y + iPosLeft.z) + iPosLeft.x;
	uint32_t iFront = g_chunkOffsetX * (g_chunkOffsetZ * iPosFront.y + iPosFront.z) + iPosFront.x;
	uint32_t iBack = g_chunkOffsetX * (g_chunkOffsetZ * iPosBack.y + iPosBack.z) + iPosBack.x;

	bool rightSolid = m_chunks[getChunkPos(rightBlock)].blocks[iRight].type != BlockType::AIR;
	bool leftSolid = m_chunks[getChunkPos(leftBlock)].blocks[iLeft].type != BlockType::AIR;
	bool topSolid = m_chunks[getChunkPos(currBlock)].blocks[iTop].type != BlockType::AIR;
	bool bottomSolid = m_chunks[getChunkPos(currBlock)].blocks[iBottom].type != BlockType::AIR;
	bool frontSolid = m_chunks[getChunkPos(frontBlock)].blocks[iFront].type != BlockType::AIR;
	bool backSolid = m_chunks[getChunkPos(backBlock)].blocks[iBack].type != BlockType::AIR;

	if (type == RayType::REMOVE)
	{
		auto& curr = m_chunks[getChunkPos(currBlock)];
		m_chunks[getChunkPos(currBlock)].blocks[iCurr].type = BlockType::AIR;

		curr.faces.erase(
			std::remove_if(
			curr.faces.begin(), curr.faces.end(),
			[&](const Face& face) {
				return face.blockID == iCurr;
			}), 
			curr.faces.end()
		);
		curr.updated = false;

		if (rightSolid)		{ setBlockFace(m_chunks[getChunkPos(rightBlock)],	iRight, Face::FaceType::LEFT); }
		if (leftSolid)		{ setBlockFace(m_chunks[getChunkPos(leftBlock)],	iLeft, Face::FaceType::RIGHT); }
		if (topSolid)		{ setBlockFace(m_chunks[getChunkPos(currBlock)],	iTop, Face::FaceType::BOTTOM); }
		if (bottomSolid)	{ setBlockFace(m_chunks[getChunkPos(currBlock)],	iBottom, Face::FaceType::TOP); }
		if (frontSolid)		{ setBlockFace(m_chunks[getChunkPos(frontBlock)],	iFront, Face::FaceType::BACK); }
		if (backSolid)		{ setBlockFace(m_chunks[getChunkPos(backBlock)],	iBack, Face::FaceType::FRONT); }
	}
	else
	{
		m_chunks[getChunkPos(currBlock)].blocks[iCurr].type = BlockType::GRASS_DIRT;

		if (rightSolid) { removeBlockFace(m_chunks[getChunkPos(rightBlock)], iRight, Face::FaceType::LEFT); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::RIGHT); }

		if (leftSolid) { removeBlockFace(m_chunks[getChunkPos(leftBlock)], iLeft, Face::FaceType::RIGHT); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::LEFT); }

		if (topSolid) { removeBlockFace(m_chunks[getChunkPos(currBlock)], iTop, Face::FaceType::BOTTOM); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::TOP); }

		if (bottomSolid) { removeBlockFace(m_chunks[getChunkPos(currBlock)], iBottom, Face::FaceType::TOP); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::BOTTOM); }

		if (frontSolid) { removeBlockFace(m_chunks[getChunkPos(frontBlock)], iFront, Face::FaceType::BACK); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::FRONT); }

		if (backSolid) { removeBlockFace(m_chunks[getChunkPos(backBlock)], iBack, Face::FaceType::FRONT); }
		else { setBlockFace(m_chunks[getChunkPos(currBlock)], iCurr, Face::FaceType::BACK); }
	}
}
/*
void Application::processRay(Engine::Ray ray)
{
	bool hit = false;

	auto type = m_keyboard[GLFW_MOUSE_BUTTON_LEFT] ? RayType::REMOVE : RayType::PLACE;
	
	float dl = 0.1f;
	float length = glm::length(ray.end - ray.start);
	glm::vec3 dir = glm::normalize(ray.end - ray.start);

	glm::ivec3 iPos = glm::ivec3(0);
	glm::vec3 currPos = {};
	glm::ivec3 currChunkPos = {};
	for (float mag = 0.0f; mag < length; mag += dl)
	{
		currPos = ray.start + mag * dir;
		currChunkPos = { 
			static_cast<int32_t>(currPos.x) / g_chunkOffsetX * g_chunkOffsetX, 
			0, 
			static_cast<int32_t>(currPos.z) / g_chunkOffsetZ * g_chunkOffsetZ };

		if (m_chunks.find(currChunkPos) == m_chunks.end())
		{
			continue;
		}
		
		iPos = static_cast<glm::ivec3>(currPos) - currChunkPos;
		uint32_t id = g_chunkOffsetX * (g_chunkOffsetX * iPos.y + iPos.z) + iPos.x;

		if (m_chunks[currChunkPos].blocks[id].type != BlockType::AIR)
		{
			if (type == RayType::PLACE)
			{
				currPos -= dl * dir;
				iPos = static_cast<glm::ivec3>(currPos) - currChunkPos;
			}

			traceRay(currPos, type);
			hit = true;
			break;
		}
	}
	
	if (m_chunks.find(currChunkPos) != m_chunks.end())
	{
		if (type == RayType::PLACE && !hit)
		{
			uint32_t id = g_chunkOffsetX * (iPos.y * g_chunkOffsetZ + iPos.z) + iPos.x;
			m_chunks[currChunkPos].blocks[id].type = BlockType::GRASS_DIRT;
			traceRay(currPos, type);
		}
	}
}
*/