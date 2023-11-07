#include <glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

using namespace Engine;

constexpr float g_cameraSpeed = 0.0125f;
constexpr float g_rayMagnitude = 3.0f;

Camera* g_camera = nullptr;
static bool g_firstMouseMove = true;

void Engine::updateCameraView(Camera& camera)
{	
	camera.view	= glm::lookAt(
		camera.pos,
		camera.front + camera.pos,
		camera.up
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

glm::mat4 Engine::getCameraView()
{
	return g_camera->view;
}

glm::vec3 Engine::getCameraPos()
{
	return g_camera->pos;
}

Ray Engine::castRay(const Camera& camera)
{
	glm::vec3 start = camera.pos;
	glm::vec3 end = start + camera.front * g_rayMagnitude;

	return { start, end };
}
