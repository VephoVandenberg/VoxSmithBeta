#pragma once

struct GLFWwindow;

namespace Engine
{
	GLFWwindow* getWindow(const char* title, size_t width, size_t height);

	void clearScreen();
	void updateScreen(GLFWwindow* window);
}
