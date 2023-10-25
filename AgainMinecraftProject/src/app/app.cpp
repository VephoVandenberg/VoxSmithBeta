#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>
#include <mutex>
#include <thread>

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
using namespace GameModule;
#endif
using namespace App;

const char* g_title = "Azamat's making Minecraft fucking again";
constexpr size_t g_width = 1240;
constexpr size_t g_height = 720;

constexpr size_t g_numberOfChunksX = 4;
constexpr size_t g_numberOfChunksZ = 4;

constexpr int32_t g_chunkOffsetX = 16;
constexpr int32_t g_chunkOffsetZ = 16;
constexpr int32_t g_chunkOffsetY = 256;

constexpr size_t g_blocksInChunk = g_chunkOffsetX * g_chunkOffsetY * g_chunkOffsetZ;
constexpr size_t g_terrainSize = g_blocksInChunk * g_numberOfChunksX * g_numberOfChunksZ;

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

	m_minBorder = { 0, 0, 0 };
	m_maxBorder = { g_numberOfChunksX * g_chunkOffsetX, 0, g_numberOfChunksZ  * g_chunkOffsetZ };

	initCamera(m_window, glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(g_chunkOffsetX * g_numberOfChunksX / 2, g_chunkOffsetY / 2 + 1, g_chunkOffsetZ  * g_numberOfChunksZ / 2));
	initShaders();
	initTextures();

#ifdef ECS
#else
	for (int32_t x = 0; x < g_numberOfChunksX; x++)
	{
		for (int32_t z = 0; z < g_numberOfChunksZ; z++)
		{
			glm::vec3 pos = { x * g_chunkOffsetX, 0, z * g_chunkOffsetZ };
			m_chunks[pos] = generateChunk(pos);
			initMeshData(m_chunks[pos]);
			generateMesh(m_chunks[pos]);
			loadChunkMesh(m_chunks[pos]);
		}
	}
#endif
}

void Application::initShaders()
{
	Engine::loadShaders(m_shaders);

	glm::mat4 projection =
		glm::perspective(
			glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);

	useShader(m_shaders[s_cubeShader]);
	setUniform4m(m_shaders[s_cubeShader], "u_projection", projection);
	setUniform4m(m_shaders[s_cubeShader], "u_view", getCameraView());

	useShader(m_shaders[s_meshShader]);
	setUniform4m(m_shaders[s_meshShader], "u_projection", projection);
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());

	useShader(m_shaders[s_rayShader]);
	setUniform4m(m_shaders[s_rayShader], "u_projection", projection);
	setUniform4m(m_shaders[s_rayShader], "u_view", getCameraView());
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
	setUniform4m(m_shaders[s_cubeShader], "u_view", getCameraView());
	
	for (const auto& component : m_components)
	{
		setUniform3f(m_shaders[s_cubeShader], "u_position", component.pos);
		Renderer::render(Renderer::Type::CUBE);
	}
#else
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());
	for (const auto& chunk : m_chunks)
	{
		drawChunk(chunk.second);
	}
#endif

	setUniform4m(m_shaders[s_rayShader], "u_view", getCameraView());
	Renderer::render(Renderer::Type::RAY);
}

void Application::onUpdate()
{
	updateCameraMove(m_keyboard);
	updateTerrain();
	
	if (m_keyboard[GLFW_MOUSE_BUTTON_LEFT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] ||
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
		Ray ray = castRay();
		Renderer::loadRayData(ray);

#ifdef ECS
#else
		processRay(ray);
				
#endif
		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
}

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

void Application::processRay(Engine::Ray ray)
{
	auto type = m_keyboard[GLFW_MOUSE_BUTTON_LEFT] ? RayType::REMOVE : RayType::PLACE;
	
	float dl = 0.1f;
	float length = glm::length(ray.end - ray.start);
	glm::vec3 dir = glm::normalize(ray.end - ray.start);

	glm::ivec3 iPos = glm::ivec3(0);
	glm::ivec3 chunkPos = {};
	for (float mag = 0.0f; mag < length; mag += dl)
	{
		glm::vec3 currPos = ray.start + mag * dir;
		int32_t posX = static_cast<int32_t>(currPos.x) / g_chunkOffsetX * g_chunkOffsetX;
		int32_t posZ = static_cast<int32_t>(currPos.z) / g_chunkOffsetZ * g_chunkOffsetZ;
		chunkPos = { posX, 0, posZ };

		if (m_chunks.find(chunkPos) == m_chunks.end())
		{
			continue;
		}
		
		iPos = static_cast<glm::ivec3>(currPos) - chunkPos;
		uint32_t id = g_chunkOffsetX * (g_chunkOffsetX * iPos.y + iPos.z) + iPos.x;

		if (m_chunks[chunkPos].blocks[id].type != BlockType::AIR)
		{
			if (type == RayType::REMOVE)
			{
				m_chunks[chunkPos].blocks[id].type = BlockType::AIR;
				removeBlock(m_chunks[chunkPos], iPos);
				ray.end = currPos;
				break;
			}
			else
			{
				iPos = currPos - dl * dir;
				currPos -= dl * dir;
				id = g_chunkOffsetX * (iPos.y * g_chunkOffsetZ + iPos.z) + iPos.x;
				m_chunks[chunkPos].blocks[id].type = BlockType::GRASS_DIRT;
				ray.end = currPos;
				addBlock(m_chunks[chunkPos], iPos);
				break;
			}
		}
	}

	generateMesh(m_chunks[chunkPos]);
	loadChunkMesh(m_chunks[chunkPos]);
}
