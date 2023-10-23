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
		Ray ray = castRay();
		Renderer::loadRayData(ray);

#ifdef ECS
#else
		glm::vec3 rayChunkPos = {
			g_chunkOffsetX * static_cast<int32_t>(ray.start.x / g_chunkOffsetX),
			0,
			g_chunkOffsetZ * static_cast<int32_t>(ray.start.z / g_chunkOffsetZ)
		};
		// Still could be rewritten
		if (m_chunks.find(rayChunkPos) != m_chunks.end())
		{
			if (rayStartInChunk(m_chunks[rayChunkPos], ray))
			{
				RayType type = m_keyboard[GLFW_MOUSE_BUTTON_1] ? RayType::REMOVE : RayType::PLACE;
				if (processRayInChunk(m_chunks[rayChunkPos], ray, type))
				{
					generateMesh(m_chunks[rayChunkPos]);

					if (rayEndInChunkBorder(m_chunks[rayChunkPos], ray) || 
						!rayEndInChunk(m_chunks[rayChunkPos], ray))
					{
						int32_t xOffset = 0;
						if (m_chunks[rayChunkPos].pos.x + 1 >= ray.end.x && m_chunks[rayChunkPos].pos.x <= ray.end.x)
						{
							xOffset = -1;
						}
						else if (m_chunks[rayChunkPos].pos.x + g_chunkOffsetX - 1 <= ray.end.x &&
							m_chunks[rayChunkPos].pos.x + g_chunkOffsetX >= ray.end.x)
						{
							xOffset = 1;
						}

						int32_t zOffset = 0;
						if (m_chunks[rayChunkPos].pos.z + 1 >= ray.end.z && m_chunks[rayChunkPos].pos.z <= ray.end.z)
						{
							zOffset = -1;
						}
						else if (m_chunks[rayChunkPos].pos.z + g_chunkOffsetZ - 1 <= ray.end.z &&
							m_chunks[rayChunkPos].pos.z + g_chunkOffsetZ >= ray.end.z)
						{
							zOffset = 1;
						}

						glm::vec3 pos = {
							g_chunkOffsetX * static_cast<int32_t>(ray.end.x / g_chunkOffsetX) + 
								xOffset * g_chunkOffsetX,
							0,
							g_chunkOffsetZ * static_cast<int32_t>(ray.end.z / g_chunkOffsetZ) + 
								zOffset * g_chunkOffsetZ
						};
						if (m_chunks.find(pos) != m_chunks.end())
						{
							generateMesh(m_chunks[pos]);

							auto less =
								pos.x < rayChunkPos.x || pos.z < rayChunkPos.z ?
								m_chunks[pos] : m_chunks[rayChunkPos];
							auto more =
								pos.x < rayChunkPos.x || pos.z < rayChunkPos.z ?
								m_chunks[rayChunkPos] : m_chunks[pos];

							updateChunkNeighbourFace(less, more);

							loadChunkMesh(m_chunks[pos]);
						}
					}

					loadChunkMesh(m_chunks[rayChunkPos]);
				}
				else if (!rayEndInChunk(m_chunks[rayChunkPos], ray))
				{
					glm::vec3 pos = {
						g_chunkOffsetX * static_cast<int32_t>(ray.end.x / g_chunkOffsetX),
						0,
						g_chunkOffsetZ * static_cast<int32_t>(ray.end.z / g_chunkOffsetZ)
					};
					if (m_chunks.find(pos) != m_chunks.end())
					{
						if (processRayInChunk(m_chunks[pos], ray, type))
						{
							generateMesh(m_chunks[pos]);
							if (rayEndInChunkBorder(m_chunks[pos], ray))
							{
								generateMesh(m_chunks[rayChunkPos]);
							}

							auto less =
								pos.x < rayChunkPos.x || pos.z < rayChunkPos.z ?
								m_chunks[pos] : m_chunks[rayChunkPos];
							auto more =
								pos.x < rayChunkPos.x || pos.z < rayChunkPos.z ?
								m_chunks[rayChunkPos] : m_chunks[pos];

							updateChunkNeighbourFace(less, more);

							if (rayEndInChunkBorder(m_chunks[pos], ray))
							{
								loadChunkMesh(m_chunks[rayChunkPos]);
							}
							loadChunkMesh(m_chunks[pos]);
						}
					}
				}
			}
		}
#endif

		m_keyboardPressed[GLFW_MOUSE_BUTTON_RIGHT] = true;
		m_keyboardPressed[GLFW_MOUSE_BUTTON_LEFT] = true;
	}
}
