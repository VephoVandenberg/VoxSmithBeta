#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	namespace Renderer
	{
		struct Vertex
		{
			glm::vec3 pos;
			glm::vec2 texCoord;
		};
		
		struct Mesh
		{
			uint32_t VAO;
			uint32_t VBO;

			size_t size;
			Vertex* vertices = nullptr;
		};

		void loadData(Mesh* mesh);
		void renderMesh(const Mesh* mesh);
	}
}