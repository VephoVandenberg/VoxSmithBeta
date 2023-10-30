#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Engine
{
	namespace Renderer
	{
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 uvw;
		};
		
		struct Mesh
		{
			uint32_t VAO;
			uint32_t VBO;

			size_t capacity;
			std::vector<Vertex> vertices;
		};

		void loadData(Mesh* mesh);
		void renderMesh(const Mesh* mesh);
	}
}