#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;

namespace Engine
{
	struct Camera
	{
		float speed;
		glm::vec3 pos;
		glm::vec3 up;
		glm::vec3 front;
		glm::mat4 view;
		float lastX;
		float lastY;
		float yaw;
		float pitch;
	};

	void initCamera(GLFWwindow* window, const glm::vec3 target, const glm::vec3 pos);
	void freeCamera();
	void updateCameraMove(const bool* keyboard);
	void updateCameraRotation(GLFWwindow* window, const double xPos, const double yPos);

	glm::mat4 getCameraView();
}