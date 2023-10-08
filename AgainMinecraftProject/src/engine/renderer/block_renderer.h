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
			QUAD
		};

		struct Buffer
		{
			uint32_t VAO;
			uint32_t VBO;
		};

		void loadCubeData();
		void loadRayData(const Ray& ray);

		void render(const Type type);
	}
}
