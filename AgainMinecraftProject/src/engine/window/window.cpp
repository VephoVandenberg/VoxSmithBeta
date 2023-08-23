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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(EXIT_FAILURE);
	}
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return window;
}

void Engine::clearScreen()
{
	glClearColor(147.0f/255.0f, 202.0f/255.0f, 237.0f/255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::updateScreen(GLFWwindow* window)
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}
