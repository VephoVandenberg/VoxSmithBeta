#pragma once

#include <glm/glm.hpp>

#include "../../engine/renderer/mesh.h"

namespace GameModule
{
	enum class BlockType
	{
		AIR,
		GRASS_DIRT,
		DIRT,
		STONE,
		SAND,
		SNOW,
	};

	enum class FaceType 
	{
		FRONT,
		BACK,
		TOP,
		BOTTOM,
		LEFT,
		RIGHT
	};

	struct Block
	{
		BlockType type;
		glm::vec3 pos;
	};

	struct Face
	{
		int32_t blockID;
		FaceType type;
		Engine::Renderer::Vertex vertices[6];
	};

}
