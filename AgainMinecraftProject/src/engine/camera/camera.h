#pragma once

#include <glm/glm.hpp>

#include "../ray/ray.h"

struct GLFWwindow;

namespace Engine
{
	struct Ray;

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

		Ray ray;
	};

	void initCamera(GLFWwindow* window, const glm::vec3 target, const glm::vec3 pos);
	void freeCamera();
	void updateCameraMove(const bool* keyboard);
	void updateCameraRotation(GLFWwindow* window, const double xPos, const double yPos);
	
	Ray castRay();
	glm::mat4 getCameraView();
	glm::vec3 getCameraPos();
}