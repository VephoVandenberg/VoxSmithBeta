#pragma once

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>

#include "../../engine/renderer/mesh.h"

constexpr uint32_t g_facePerCube = 6;
constexpr uint32_t g_vertexPerFace = g_facePerCube;

namespace GameModule
{
	enum class BlockType : int8_t
	{
		AIR,
		GRASS,
		DIRT,
		STONE,
		SAND,
		SNOW,
		WATER
	};

	struct Block
	{
		BlockType type;
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
	};

	struct EnumHash
	{
		template <typename T>
		uint8_t operator()(T t) const { return static_cast<uint8_t>(t); }
	};
}

