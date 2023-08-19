#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

#include "../engine/window/window.h"
#include "../engine/renderer/cube_renderer.h"

#include "app.h"

using namespace Engine;
using namespace App;

const char* g_title = "Azamat's making Minecraft fucking again";
constexpr size_t g_width = 1240;
constexpr size_t g_height = 720;

Application::Application()
	: m_isRunning(true)
{
	init();
}

void Application::init()
{
	m_window = getWindow(g_title, g_width, g_height);

	m_camera.lastX = g_width / 2;
	m_camera.lastY = g_height / 2;
	initCamera(m_camera, m_window, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
	initShaders();
	initTextures();
}

void Application::initShaders()
{

	float vertices[] = {
		// back
		0, 0, 0, 	0, 0,
		1, 0, 0,	1, 0,
		0, 1, 0,	0, 1,

		1, 0, 0,	1, 0,
		1, 1, 0,	1, 1,
		0, 1, 0,	0, 1,

		// front
		0, 0, 1, 	0, 0,
		1, 0, 1,	1, 0,
		0, 1, 1,	0, 1,

		1, 0, 1,	1, 0,
		1, 1, 1,	1, 1,
		0, 1, 1,	0, 1,

		// top
		0, 1, 1, 	0, 0,
		1, 1, 1,	1, 0,
		0, 1, 0,	0, 1,

		1, 1, 1,	1, 0,
		1, 1, 0,	1, 1,
		0, 1, 0,	0, 1,

		// bottom
		0, 0, 1, 	0, 0,
		1, 0, 1,	1, 0,
		0, 0, 0,	0, 1,

		1, 0, 1,	1, 0,
		1, 0, 0,	1, 1,
		0, 0, 0,	0, 1,

		// left
		0, 0, 0, 	0, 0,
		0, 0, 1,	1, 0,
		0, 1, 0,	0, 1,

		0, 0, 1,	1, 0,
		0, 1, 1,	1, 1,
		0, 1, 0,	0, 1,

		// right
		1, 0, 1, 	0, 0,
		1, 0, 0,	1, 0,
		1, 1, 1,	0, 1,

		1, 0, 0,	1, 0,
		1, 1, 0,	1, 1,
		1, 1, 1,	0, 1,
	};

	Shader shader;
	initShader(shader, "shaders/cube_shader.vert", "shaders/cube_shader.frag");
	m_shaders.insert({ "cube", shader });
	useShader(m_shaders["cube"]);
	glm::mat4 projection =
		glm::perspective(
			glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);
	Renderer::loadData(vertices, sizeof(vertices));
	useShader(m_shaders["cube"]);
	setUniform4m(m_shaders["cube"], "u_projection", projection);
	setUniform4m(m_shaders["cube"], "u_view", m_camera.view);
}

void Application::initTextures()
{

}

void Application::run()
{
	Texture texture;
	initTexture(texture, "textures/grass.png");
	useTexture(texture);

	while (m_isRunning)
	{
		clearScreen();
		setUniform4m(m_shaders["cube"], "u_view", m_camera.view);

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

	updateCameraMove(m_keyboard);
}

void Application::onRender()
{
	Renderer::renderCube();
}

void Application::onUpdate()
{
	
}
