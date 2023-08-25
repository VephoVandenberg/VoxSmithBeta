#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

#include "../engine/window/window.h"
#include "../engine/shader/shader_list.h"

#include "../engine/ray/ray.h"
#include "../engine/renderer/block_renderer.h"

#include "../modules/chunk/chunk.h"

#include "app.h"

using namespace Engine;
using namespace GameModule;
using namespace App;

const char* g_title = "Azamat's making Minecraft fucking again";
constexpr size_t g_width = 1240;
constexpr size_t g_height = 720;

constexpr size_t numberOfChunksX = 3;
constexpr size_t numberOfChunksZ = 3;

constexpr glm::vec3 g_chunkSize = { 8, 8, 8 };

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

	for (uint32_t x = 0; x < numberOfChunksX; x++)
	{
		for (uint32_t z = 0; z < numberOfChunksZ; z++)
		{
			m_chunks.emplace_back(generateChunk({ x, 0, z }));
			generateMesh(m_chunks.back());
			loadChunkMesh(m_chunks.back());
		}
	}
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

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		m_keyboard[GLFW_MOUSE_BUTTON_1] = true;
	}
	else
	{
		m_keyboard[GLFW_MOUSE_BUTTON_1] = false;
	}
}

void Application::onRender()
{
	setUniform4m(m_shaders[s_meshShader], "u_view", getCameraView());
	for (const auto& chunk: m_chunks)
	{
		drawChunk(chunk);
	}

	setUniform4m(m_shaders[s_rayShader], "u_view", getCameraView());
	Renderer::render(Renderer::Type::RAY);
}

void Application::onUpdate()
{
	updateCameraMove(m_keyboard);

	if (m_keyboard[GLFW_MOUSE_BUTTON_1])
	{
		Ray ray = castRay();
		Renderer::loadRayData(ray);
	}
}
