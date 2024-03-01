#include <glad/glad.h>

#include "mesh.h"

using namespace Engine::Renderer;

void Engine::Renderer::initBuffer(MeshBuffer& buffer)
{
	glGenVertexArrays(1, &buffer.VAO);
	glGenBuffers(1, &buffer.VBO);

	glBindVertexArray(buffer.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.VBO);

	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
}

void Engine::Renderer::initUBufferLM(UBuffer& uBuffer)
{
	glGenBuffers(1, &uBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4x4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Engine::Renderer::useUBufferLM(UBuffer& uBuffer)
{
	glBindBuffer(GL_UNIFORM_BUFFER, uBuffer);
}

void Engine::Renderer::updateUBufferLM(UBuffer& uBuffer, const std::vector<glm::mat4>& lightMatrices)
{
	useUBufferLM(uBuffer);
	for (uint32_t i = 0; i < lightMatrices.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Engine::Renderer::renderMesh(const MeshBuffer& buffer)
{
	glBindVertexArray(buffer.VAO);
	glDrawArrays(GL_TRIANGLES, 0, buffer.nVertices);
	glBindVertexArray(0);
}

void Engine::Renderer::updateMesh(MeshBuffer& buffer, const Mesh& mesh)
{
	if (!buffer.VAO)
	{
		initBuffer(buffer);
	}

	buffer.nVertices = mesh.size();
	glBindVertexArray(buffer.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, buffer.VBO);
	glBufferData(GL_ARRAY_BUFFER, mesh.size() * sizeof(Vertex), mesh.data(), GL_DYNAMIC_DRAW);
}

void Engine::Renderer::deleteMesh(MeshBuffer& mesh)
{
	glDeleteBuffers(1, &mesh.VBO);
	glDeleteVertexArrays(1, &mesh.VAO);
}


void Engine::Renderer::enableCulling()
{
	glEnable(GL_CULL_FACE);
}

void Engine::Renderer::disableCulling()
{
	glDisable(GL_CULL_FACE);
}

