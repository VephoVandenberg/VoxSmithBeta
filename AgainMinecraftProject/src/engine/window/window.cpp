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
	setViewport(width, height);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}

void Engine::clearScreen()
{
	glClearColor(147.0f/255.0f, 202.0f/255.0f, 237.0f/255.0f, 1.0f);
	clearBuffers();
}

void Engine::clearBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Engine::clearDepthBuff()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Engine::enableCulling()
{
	//glCullFace(GL_FRONT);
}

void Engine::disableCulling()
{
	//glCullFace(GL_BACK);
}

void Engine::setViewport(const size_t width, const size_t height)
{
	glViewport(0, 0, width, height);
}

void Engine::updateScreen(GLFWwindow* window)
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}
