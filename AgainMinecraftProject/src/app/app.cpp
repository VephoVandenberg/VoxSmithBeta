#include "../engine/window/window.h"

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
	while (m_isRunning)
	{
		clearScreen();
		sfwapBuffersAndPollEvents(m_window);
	}
}
