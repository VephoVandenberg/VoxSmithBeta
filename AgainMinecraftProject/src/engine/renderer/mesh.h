#pragma once

#include <vector>

#include <glm/glm.hpp>

namespace Engine
{
	namespace Renderer
	{
		struct Vertex
		{
			// x : 4 bits, y: 8 bits, z : 4 bits 
			// int16_t localData
			glm::vec3 pos;
			int8_t coordInd;
			int8_t texId;
		};
		
		struct Mesh
		{
			uint32_t VAO;
			uint32_t VBO;

			std::vector<Vertex> vertices;
		};

		void loadData(Mesh* mesh);
		void renderMesh(const Mesh* mesh);
	}
}