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
		0, 0,	0, 0,
		1, 0,	1, 0,
		0, 1,	0, 1,

		1, 0,	1, 0,
		1, 1,	1, 1,
		0, 1,	0, 1
	};

	Shader shader;

	initShader(shader, "shaders/base_shader.vert", "shaders/base_shader.frag");
	initRenderer(vertices, sizeof(vertices));
	useShader(shader);

	Texture texture;
	initTexture(texture, "textures/grass.png");
	useTexture(texture);

	while (m_isRunning)
	{
		clearScreen();

		render();

		updateScreen(m_window);
	}
}

void Application::handleInput()
{

}

void Application::onRender()
{

}

void Application::onUpdate()
{

}
