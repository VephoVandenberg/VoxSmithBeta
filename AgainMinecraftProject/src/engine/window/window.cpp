#include <iostream>
#include <stdlib.h>

#include <GLAD/glad.h>
#include <glfw3.h>

#include "window.h"

GLFWwindow* Engine::getWindow(const char* title, size_t width, size_t height)
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		exit(EXIT_FAILURE);
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(window);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}

	return window;
}

void Engine::clearScreen()
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Engine::sfwapBuffersAndPollEvents(GLFWwindow* window)
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}