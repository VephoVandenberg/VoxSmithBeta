#include "../engine/window/window.h"
#include "../engine/shader/shader.h"
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
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,

		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	Shader shader;
	initShader(shader, "shaders/base_shader.vert", "shaders/base_shader.frag");

	initRenderer(vertices, sizeof(vertices));

	while (m_isRunning)
	{
		clearScreen();

		useShader(shader);
		render();

		update(m_window);
	}
}
