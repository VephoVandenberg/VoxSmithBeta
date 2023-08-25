#include <stdint.h>
#include <glad/glad.h>

#include "../ray/ray.h"

#include "block_renderer.h"

using namespace Engine::Renderer;

Buffer g_buffer;

// This might not be needed anymore
constexpr float g_cubeVertices[] = {
	// Vertices, TexCoords

	// back
	0, 0, 0, 	0, 0,
	1, 0, 0,	1, 0,
	0, 1, 0,	0, 1,

	1, 0, 0,	1, 0,
	1, 1, 0,	1, 1,
	0, 1, 0,	0, 1,

	// front
	0, 0, 1, 	0, 0,
	1, 0, 1,	1, 0,
	0, 1, 1,	0, 1,

	1, 0, 1,	1, 0,
	1, 1, 1,	1, 1,
	0, 1, 1,	0, 1,

	// top
	0, 1, 1, 	0, 0,
	1, 1, 1,	1, 0,
	0, 1, 0,	0, 1,

	1, 1, 1,	1, 0,
	1, 1, 0,	1, 1,
	0, 1, 0,	0, 1,

	// bottom
	0, 0, 1, 	0, 0,
	1, 0, 1,	1, 0,
	0, 0, 0,	0, 1,

	1, 0, 1,	1, 0,
	1, 0, 0,	1, 1,
	0, 0, 0,	0, 1,

	// left
	0, 0, 0, 	0, 0,
	0, 0, 1,	1, 0,
	0, 1, 0,	0, 1,

	0, 0, 1,	1, 0,
	0, 1, 1,	1, 1,
	0, 1, 0,	0, 1,

	// right
	1, 0, 1, 	0, 0,
	1, 0, 0,	1, 0,
	1, 1, 1,	0, 1,

	1, 0, 0,	1, 0,
	1, 1, 0,	1, 1,
	1, 1, 1,	0, 1,
};

constexpr float g_lineWidth = 5.0f;

void Engine::Renderer::loadCubeData(Buffer& buffer, const float* vertices, bool hasColor, bool hasTexture)
{
	// This function might not be needed at all
}

void Engine::Renderer::loadRayData(const Ray& ray)
{
	const float vertices[] = {
		ray.start.x, ray.start.y, ray.start.z,
		ray.end.x, ray.end.y, ray.end.z
	};

	glGenVertexArrays(1, &g_buffer.VAO);
	glGenBuffers(1, &g_buffer.VBO);

	glBindVertexArray(g_buffer.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_buffer.VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glLineWidth(g_lineWidth);
}

void Engine::Renderer::render(const Type type)
{
	glBindVertexArray(g_buffer.VAO);
	switch (type)
	{
	case Type::CUBE:
		glDrawArrays(GL_TRIANGLES, 0, 36);
		break;

	case Type::QUAD:
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	
	case Type::RAY:
		glDrawArrays(GL_LINES, 0, 2);
		break;
	}
}
