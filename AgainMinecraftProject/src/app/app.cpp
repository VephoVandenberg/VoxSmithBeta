#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

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

constexpr size_t g_numberOfChunksX = 2;
constexpr size_t g_numberOfChunksZ = 2;

constexpr size_t g_chunkOffsetX = 16;
constexpr size_t g_chunkOffsetZ = 16;
constexpr size_t g_chunkOffsetY = 16;

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

	initCamera(m_window, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
	initShaders();
	initTextures();

#ifdef ECS
	Renderer::loadCubeData();
	for (int32_t entity = 0; entity < g_terrainSize; entity++)
	{
		m_availableEntities.push(entity);
	}

	for (int32_t y = 0; y < g_chunkOffsetY; y++)
	{
		for (int32_t z = 0; z < g_chunkOffsetZ; z++)
		{
			for (int32_t x = 0; x < g_chunkOffsetX; x++)
			{
				auto type = GameModule::BlockType::GRASS_DIRT;
				m_components.push_back({ type, { x, y, z } });
				m_entityToIndex[m_availableEntities.front()] = m_components.size();
			}
		}
	}
#else
	for (uint32_t x = 0; x < g_numberOfChunksX; x++)
	{
		for (uint32_t z = 0; z < g_numberOfChunksZ; z++)
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

	if (m_keyboard[GLFW_MOUSE_BUTTON_LEFT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] ||
		m_keyboard[GLFW_MOUSE_BUTTON_RIGHT] && !m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT])
	{
#ifdef _DEBUG
		Ray ray = castRay();
		Renderer::loadRayData(ray);
#endif

#ifdef ECS
#else
		auto type = m_keyboard[GLFW_MOUSE_BUTTON_LEFT] ? RayType::REMOVE : RayType::PLACE;
		glm::vec3 rayStartChunk = {
			g_chunkOffsetX * static_cast<int32_t>(ray.start.x / g_chunkOffsetX),
			0,
			g_chunkOffsetZ * static_cast<int32_t>(ray.start.z / g_chunkOffsetZ)
		};
		glm::vec3 rayEndChunk = {
			g_chunkOffsetX * static_cast<int32_t>(ray.end.x / g_chunkOffsetX),
			0,
			g_chunkOffsetZ * static_cast<int32_t>(ray.end.z / g_chunkOffsetZ)
		};
		
		if (m_chunks.find(rayStartChunk) == m_chunks.end() ||
			m_chunks.find(rayEndChunk) == m_chunks.end())
		{
			return;
		}
				
		if (processRayInChunk(m_chunks[rayStartChunk], ray, type))
		{
			glm::vec3 chunkToBorderX = rayStartChunk;
			if (rayEndInBorderX(m_chunks[rayStartChunk], ray))
			{
				int32_t offset =
					rayEndInBorderXPos(m_chunks[rayStartChunk], ray) ?
					static_cast<int32_t>(g_chunkOffsetX) : -static_cast<int32_t>(g_chunkOffsetX);
				chunkToBorderX.x += offset;

				if (m_chunks.find(chunkToBorderX) != m_chunks.end())
				{
					auto& less =
						chunkToBorderX.x < rayStartChunk.x ?
						m_chunks[chunkToBorderX] : m_chunks[rayStartChunk];
					auto& more =
						chunkToBorderX.x > rayStartChunk.x ?
						m_chunks[chunkToBorderX] : m_chunks[rayStartChunk];

					updateChunkNeighbourFace(less, more);
					generateMesh(m_chunks[chunkToBorderX]);
					loadChunkMesh(m_chunks[chunkToBorderX]);
				}
			}

			glm::vec3 chunkToBorderZ = rayStartChunk;
			if (rayEndInBorderZ(m_chunks[rayStartChunk], ray))
			{
				int32_t offset =
					rayEndInBorderZPos(m_chunks[rayStartChunk], ray) ?
					static_cast<int32_t>(g_chunkOffsetZ) : -static_cast<int32_t>(g_chunkOffsetZ);
				chunkToBorderZ.z += offset;

				if (m_chunks.find(chunkToBorderZ) != m_chunks.end())
				{
					auto& less =
						chunkToBorderZ.z < rayStartChunk.z ?
						m_chunks[chunkToBorderZ] : m_chunks[rayStartChunk];
					auto& more =
						chunkToBorderZ.z > rayStartChunk.z ?
						m_chunks[chunkToBorderZ] : m_chunks[rayStartChunk];

					updateChunkNeighbourFace(less, more);
					generateMesh(m_chunks[chunkToBorderZ]);
					loadChunkMesh(m_chunks[chunkToBorderZ]);
				}
			}
			generateMesh(m_chunks[rayStartChunk]);
			loadChunkMesh(m_chunks[rayStartChunk]);
		}
		else if (processRayInChunk(m_chunks[rayEndChunk], ray, type))
		{
 			glm::vec3 endChunkBorderX = rayEndChunk;
			if (rayEndInBorderX(m_chunks[endChunkBorderX], ray))
			{
				int32_t offset =
					rayEndInBorderXPos(m_chunks[rayEndChunk], ray) ?
					static_cast<int32_t>(g_chunkOffsetX) : -static_cast<int32_t>(g_chunkOffsetX);
				endChunkBorderX.x += offset;

				if (m_chunks.find(endChunkBorderX) != m_chunks.end())
				{
					auto& less =
						endChunkBorderX.x < rayEndChunk.x ?
						m_chunks[endChunkBorderX] : m_chunks[rayEndChunk];
					auto& more =
						endChunkBorderX.x > rayEndChunk.x ?
						m_chunks[endChunkBorderX] : m_chunks[rayEndChunk];

					updateChunkNeighbourFace(less, more);
					generateMesh(m_chunks[endChunkBorderX]);
					loadChunkMesh(m_chunks[endChunkBorderX]);
				}
			}

			glm::vec3 endChunkBorderZ = rayEndChunk;
			if (rayEndInBorderZ(m_chunks[endChunkBorderZ], ray))
			{
				int32_t offset = 
					rayEndInBorderZPos(m_chunks[rayEndChunk], ray) ?
					static_cast<int32_t>(g_chunkOffsetZ) : -static_cast<int32_t>(g_chunkOffsetZ);
				endChunkBorderZ.z += offset;

				if (m_chunks.find(endChunkBorderZ) != m_chunks.end())
				{
					auto& less =
						endChunkBorderZ.z < rayEndChunk.z ?
						m_chunks[endChunkBorderZ] : m_chunks[rayEndChunk];
					auto& more =
						endChunkBorderZ.z > rayEndChunk.z ?
						m_chunks[endChunkBorderZ] : m_chunks[rayEndChunk];

					updateChunkNeighbourFace(less, more);
					generateMesh(m_chunks[endChunkBorderZ]);
					loadChunkMesh(m_chunks[endChunkBorderZ]);
				}
			}
			generateMesh(m_chunks[rayEndChunk]);
			loadChunkMesh(m_chunks[rayEndChunk]);
		}
				
#endif
		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
}
