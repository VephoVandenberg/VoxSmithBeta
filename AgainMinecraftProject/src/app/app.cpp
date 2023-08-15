#include <glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <functional>

#include "../engine/window/window.h"
#include "../engine/renderer/renderer.h"

#include "app.h"

using namespace Engine;
using namespace App;

const char* g_title = "Azamat's Minecraft fucking again";
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
}

void Application::run()
{
	float vertices[] = {
		0, 0, 0, 	0, 0,
		1, 0, 0,	1, 0,
		0, 1, 0,	0, 1,

		1, 0, 0,	1, 0,
		1, 1, 0,	1, 1,
		0, 1, 0,	0, 1
	};

	Shader shader;

	m_camera.lastX = g_width / 2;
	m_camera.lastY = g_height / 2;
	initCamera(m_camera, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 3.0f));
	glfwSetCursorPosCallback(m_window, updateCameraRotation);
	initShader(shader, "shaders/base_shader.vert", "shaders/base_shader.frag");
	glm::mat4 projection = 
		glm::perspective(
			glm::radians(45.0f), static_cast<float>(g_width) / static_cast<float>(g_height), 0.1f, 100.0f);
	Renderer::initRenderer(vertices, sizeof(vertices));
	useShader(shader);
	setUniform4m(shader, "u_projection", projection);
	setUniform4m(shader, "u_view", m_camera.view);

	Texture texture;
	initTexture(texture, "textures/grass.png");
	useTexture(texture);

	while (m_isRunning)
	{
		clearScreen();
		setUniform4m(shader, "u_view", m_camera.view);

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
	Renderer::render();
}

void Application::onUpdate()
{
	
}
