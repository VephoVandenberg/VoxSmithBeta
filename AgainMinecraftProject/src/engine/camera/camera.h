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

	void updateCameraView(Camera& camera);
	void updateCameraRotation(GLFWwindow* window, const double xPos, const double yPos);
	
	Ray castRay(const Camera& camera);
}