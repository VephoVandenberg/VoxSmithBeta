#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Engine
{
	namespace Renderer
	{
		struct Vertex
		{
			// x : 5 bits, y: 9 bits, z : 5 bits (19 bits)
			// coordInd : 2 bits, texId : 4 bits (6 bits)
			// Total 25 bits
			int32_t data;
		};
		
		struct Mesh
		{
			uint32_t VAO;
			uint32_t VBO;

			std::vector<Vertex> vertices;
		};

		void loadData(Mesh* mesh);
		void renderMesh(const Mesh* mesh);
		void deleteMesh(Mesh* mesh);
	}
}