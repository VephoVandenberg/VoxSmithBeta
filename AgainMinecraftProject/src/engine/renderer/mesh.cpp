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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribIPointer(1, 1, GL_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, coordInd));
	glEnableVertexAttribArray(1);

	glVertexAttribIPointer(2, 1, GL_BYTE, sizeof(Vertex), (void*)offsetof(Vertex, texId));
	glEnableVertexAttribArray(2);
}

void Engine::Renderer::renderMesh(const Mesh* mesh)
{
	glBindVertexArray(mesh->VAO);
	glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
}
