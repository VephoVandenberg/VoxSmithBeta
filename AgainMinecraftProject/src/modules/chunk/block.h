#pragma once

#include <glm/glm.hpp>

#include <unordered_map>

#include "../../engine/renderer/mesh.h"

constexpr uint32_t g_facePerCube = 6;

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

	struct Block
	{
		BlockType	type;
		glm::vec3	pos;
	};

	struct Face
	{
		enum class FaceType
		{
			FRONT = 0,
			BACK,
			TOP,
			BOTTOM,
			LEFT,
			RIGHT
		} type;

		int32_t						blockID;
		Engine::Renderer::Vertex	vertices[6];
	};

	struct EnumHash
	{
		template <typename T>
		size_t operator()(T t) const { return static_cast<size_t>(t); }
	};
}

