#pragma once

namespace Engine
{
	struct Ray;

	namespace Renderer
	{
		enum class Type
		{
			RAY,
			CUBE,
			CUBE_LINES,
			QUAD,
			PLAYER
		};

		struct Buffer
		{
			uint32_t VAO;
			uint32_t VBO;
		};

		struct IBuffer
		{
			uint32_t VAO;
			uint32_t IBO;
			uint32_t VBO;
		};

		void loadCubeData();
		void loadQuadData();
		void loadPlayerOutlineData();
		void loadRayData(const Ray& ray);

		void render(const Type type);
	}
}
