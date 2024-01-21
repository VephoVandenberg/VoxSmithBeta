#pragma once

#include <vector>
#include <unordered_map>
#include <array>

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
		using Mesh = std::vector<Vertex>;
		
		struct Buffer
		{
			uint32_t nVertices;

			uint32_t VAO;
			uint32_t VBO;

			bool active = false;
		};

		template<uint32_t N>
		struct MeshPool
		{
			std::array<Buffer, N> meshes;
		};

		void initBuffer(Buffer& buffer);
		void updateMesh(Buffer& buffer, const Mesh& mesh);
		void renderMesh(const Buffer& buffer);
		void deleteMesh(Buffer& buffer);
	}
}