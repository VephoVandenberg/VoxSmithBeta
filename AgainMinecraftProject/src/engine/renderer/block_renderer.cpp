#include <stdint.h>
#include <glad/glad.h>

#include "block_renderer.h"

using namespace Engine::Renderer;

static uint32_t VAO;
static uint32_t VBO;

void Engine::Renderer::loadData()
{
	const float vertices[] = {
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

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3  * sizeof(float)));
}

void Engine::Renderer::renderQuad()	
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Engine::Renderer::renderCube()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
