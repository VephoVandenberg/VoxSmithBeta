#include <glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

using namespace Engine;

constexpr float g_cameraSpeed = 0.00625f;
constexpr float g_rayMagnitude = 6.0f;

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

}

Ray Engine::castRay(const Camera& camera)
{
	glm::vec3 start = camera.pos;
	glm::vec3 end = start + camera.front * g_rayMagnitude;

	return { start, end };
}
