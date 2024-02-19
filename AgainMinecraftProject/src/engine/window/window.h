#pragma once

struct GLFWwindow;

namespace Engine
{
	GLFWwindow* getWindow(const char* title, size_t width, size_t height);

	void		clearScreen();
	void		clearBuffers();
	void		setViewport(const size_t width, const size_t height);
	void		updateScreen(GLFWwindow* window);
	void		enableCulling();
	void		disableCulling();
}
