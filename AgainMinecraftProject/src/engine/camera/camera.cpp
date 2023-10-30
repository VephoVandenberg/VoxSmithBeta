#include <glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

using namespace Engine;

constexpr float g_cameraSpeed = 0.0125f;
constexpr float g_rayMagnitude = 3.0f;

static Camera* g_camera = nullptr;

static bool g_firstMouseMove = true;

void Engine::initCamera(GLFWwindow* window, const glm::vec3 target, const glm::vec3 pos)
{
	if (g_camera)
	{
		return;
	}

	g_camera = new Camera;

	g_camera->lastX = 620;
	g_camera->lastY = 360;
	g_camera->yaw = -90.0f;
	g_camera->pitch = 0.0f;
	g_camera->pos = pos;
	g_camera->speed = g_cameraSpeed;
	g_camera->front = target;
	g_camera->up = { 0.0f, 1.0f, 0.0f };
	g_camera->view = glm::lookAt(g_camera->pos, g_camera->front, g_camera->up);

	glfwSetCursorPosCallback(window, updateCameraRotation);
}

void Engine::freeCamera()
{
	delete g_camera;
}

void Engine::updateCameraMove(const bool* keyboard)
{
	if (!g_camera)
	{
		return;
	}

	if (keyboard[GLFW_KEY_W])
	{
		g_camera->pos += g_camera->front * g_camera->speed;
	}

	if (keyboard[GLFW_KEY_S])
	{
		g_camera->pos -= g_camera->front * g_camera->speed;
	}

	if (keyboard[GLFW_KEY_A])
	{
		g_camera->pos -= glm::normalize(glm::cross(g_camera->front,  g_camera->up)) * g_camera->speed;
	}

	if (keyboard[GLFW_KEY_D])
	{
		g_camera->pos += glm::normalize(glm::cross(g_camera->front, g_camera->up)) * g_camera->speed;
	}

	g_camera->view = glm::lookAt(
		g_camera->pos,
		g_camera->front + g_camera->pos,
		g_camera->up
	);
}

void Engine::updateCameraRotation(GLFWwindow* window, const double xPos, const double yPos)
{
	if (!g_camera)
	{
		return;
	}

	if (g_firstMouseMove)
	{
		g_camera->lastX = xPos;
		g_camera->lastY = yPos;
		g_firstMouseMove = false;
	}

	float xoffset = xPos - g_camera->lastX;
	float yoffset = g_camera->lastY - yPos;
	g_camera->lastX = xPos;
	g_camera->lastY = yPos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	g_camera->yaw += xoffset;
	g_camera->pitch += yoffset;

	if (g_camera->pitch > 89.0f)
	{
		g_camera->pitch = 89.0f;
	}
	if (g_camera->pitch < -89.0f)
	{
		g_camera->pitch = -89.0f;
	}

	glm::vec3 direction;
	direction.x = cos(glm::radians(g_camera->yaw)) * cos(glm::radians(g_camera->pitch));
	direction.y = sin(glm::radians(g_camera->pitch));
	direction.z = sin(glm::radians(g_camera->yaw)) * cos(glm::radians(g_camera->pitch));
	g_camera->front = glm::normalize(direction);
}

glm::mat4 Engine::getCameraView()
{
	return g_camera->view;
}

glm::vec3 Engine::getCameraPos()
{
	return g_camera->pos;
}

Ray Engine::castRay()
{
	glm::vec3 start = g_camera->pos;
	glm::vec3 end = start + g_camera->front * g_rayMagnitude;

	return { start, end };
}
