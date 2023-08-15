#include <glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

using namespace Engine;

constexpr float g_cameraSpeed = 0.005f;

static Camera* g_camera = nullptr;

static bool g_firstMouseMove = true;

void Engine::initCamera(Camera& camera, const glm::vec3 target, const glm::vec3 pos)
{
	camera.pos = pos;
	camera.speed = g_cameraSpeed;
	camera.front = target;
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.view = glm::lookAt(camera.pos, camera.front, camera.up);

	g_camera = &camera;
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
