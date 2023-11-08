#include <stdint.h>
#include <glad/glad.h>

#include "../ray/ray.h"

#include "block_renderer.h"

using namespace Engine::Renderer;

static IBuffer g_cubeBuff;
static Buffer g_rayBuff;

// This might not be needed anymore
constexpr float g_cubeOutlineVerts[] = {
	0, 0, 0, // -x -y -z (0)
	0, 1, 0, // -x +y -z (1)
	1, 1, 0, // +x +y -z (2)
	1, 0, 0, // +x -y -z (3)
	0, 0, 1, // -x -y +z (4)
	0, 1, 1, // -x +y +z (5)
	1, 1, 1, // +x +y +z (6)
	1, 0, 1, // +x -y +z (7)
};

constexpr uint32_t g_cubeOutlineInds[] = {
	0, 1,	1, 2,	2, 3,	3, 0,	// front
	4, 5,	5, 6,	6, 7,	7, 4,	// back
	0, 4,	1, 5,					// left
	3, 7,	2, 6					// right
};

constexpr float g_lineWidth = 3.0f;

void Engine::Renderer::loadCubeData()
{
	// This function might not be needed at all
	glGenVertexArrays(1, &g_cubeBuff.VAO);
	glGenBuffers(1, &g_cubeBuff.VBO);
	glGenBuffers(1, &g_cubeBuff.IBO);

	glBindVertexArray(g_cubeBuff.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, g_cubeBuff.VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_cubeBuff.IBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(g_cubeOutlineVerts), g_cubeOutlineVerts, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_cubeOutlineInds), g_cubeOutlineInds, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glLineWidth(g_lineWidth);
}

void Engine::Renderer::loadRayData(const Ray& ray)
{
	const float vertices[] = {
		ray.start.x, ray.start.y, ray.start.z,
		ray.end.x, ray.end.y, ray.end.z
	};

	glGenVertexArrays(1, &g_rayBuff.VAO);
	glGenBuffers(1, &g_rayBuff.VBO);

	glBindVertexArray(g_rayBuff.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_rayBuff.VBO);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glLineWidth(g_lineWidth);
}

void Engine::Renderer::render(const Type type)
{
	switch (type)
	{
	case Type::CUBE:
		glBindVertexArray(g_cubeBuff.VAO);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		break;

	case Type::QUAD:
		glDrawArrays(GL_TRIANGLES, 0, 6);
		break;
	
	case Type::RAY:
		glBindVertexArray(g_rayBuff.VAO);
		glDrawArrays(GL_LINES, 0, 2);
		break;
	}
}
