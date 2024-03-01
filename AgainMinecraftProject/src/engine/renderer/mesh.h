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
		
		struct MeshBuffer
		{
			uint32_t nVertices;

			uint32_t VAO;
			uint32_t VBO;

			bool active = false;
		};

		template<uint32_t N>
		struct BufferPool
		{
			std::array<MeshBuffer, N> buffers;
			uint32_t activeCounter = 0;
		};

		using UBuffer = uint32_t;

		void initBuffer(MeshBuffer& buffer);
		void updateMesh(MeshBuffer& buffer, const Mesh& mesh);
		void renderMesh(const MeshBuffer& buffer);
		void deleteMesh(MeshBuffer& buffer);

		void initUBufferLM(UBuffer& buffer);
		void useUBufferLM(UBuffer& buffer);
		void updateUBufferLM(UBuffer& buffer, const std::vector<glm::mat4>& lightMatrices);

		void enableCulling();
		void disableCulling();
	}
}