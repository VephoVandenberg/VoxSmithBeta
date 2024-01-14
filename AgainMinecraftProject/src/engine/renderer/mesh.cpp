#include <glad/glad.h>

#include "mesh.h"

using namespace Engine::Renderer;

void Engine::Renderer::loadData(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->VAO);
	glGenBuffers(1, &mesh->VBO);

	glBindVertexArray(mesh->VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), mesh->vertices.data(), GL_STATIC_DRAW);

	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)0);
	 glEnableVertexAttribArray(0);
}

void Engine::Renderer::renderMesh(const Mesh* mesh)
{
	glBindVertexArray(mesh->VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
}

void Engine::Renderer::deleteMesh(Mesh* mesh)
{
	glDeleteBuffers(1, &mesh->VBO);
	glDeleteVertexArrays(1, &mesh->VAO);
}
