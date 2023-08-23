#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

#include "../engine/window/window.h"
#include "../engine/shader/shader_list.h"
#include "../engine/renderer/block_renderer.h"

#include "../modules/chunk/chunk.h"

#include "app.h"

using namespace Engine;
using namespace GameModule;
using namespace App;

const char* g_title = "Azamat's making Minecraft fucking again";
constexpr size_t g_width = 1240;
constexpr size_t g_height = 720;

constexpr size_t numberOfChunksX = 1;
constexpr size_t numberOfChunksZ = 1;

constexpr glm::vec3 g_chunkSize = { 8, 8, 8 };

Application::Application()
	: m_isRunning(true)
{
	init();
}

void Application::init()
{
	m_window = getWindow(g_title, g_width, g_height);

	initCamera(m_window, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
	initShaders();
	initTextures();

	for (uint32_t x = 0; x < numberOfChunksX; x++)
	{
		for (uint32_t z = 0; z < numberOfChunksZ; z++)
		{
			m_chunks.emplace_back(generateChunk({ x, 0, z }));
			generateMesh(&m_chunks.back());
		}
	}

	for (uint32_t x = 0; x < g_chunkSize.x; x++)
	{
		for (uint32_t y = 0; y < g_chunkSize.y; y++)
		{
			for (uint32_t z = 0; z < g_chunkSize.z; z++)
			{
				m_blockPositions.push_back({ x, y, z });
			}
		}
	}
}

void Application::initShaders()
{
	Engine::loadShaders(m_shaders);

	glm::mat4 projection =
		glm::perspective(
			glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);
	Renderer::loadData();

	useShader(m_shaders[s_cubeShader]);
	setUniform4m(m_shaders[s_cubeShader], "u_projection", projection);
	setUniform4m(m_shaders[s_cubeShader], "u_view", getCameraView());

	useShader(m_shaders[s_meshShader]);
	setUniform4m(m_shaders[s_meshShader], "u_projection", projection);
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());
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
}

void Application::onRender()
{
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());
	for (auto& chunk: m_chunks)
	{
		drawChunk(&chunk);
	}
}

void Application::onUpdate()
{
	updateCameraMove(m_keyboard);
}
