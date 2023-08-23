#include <glad/glad.h>

#include "mesh.h"

using namespace Engine::Renderer;

void Engine::Renderer::loadData(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->VAO);
	glGenBuffers(1, &mesh->VBO);

	glBindVertexArray(mesh->VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh->size, mesh->vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);
}

void Engine::Renderer::renderMesh(const Mesh* mesh)
{
	glBindVertexArray(mesh->VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh->size / sizeof(Vertex));
}
