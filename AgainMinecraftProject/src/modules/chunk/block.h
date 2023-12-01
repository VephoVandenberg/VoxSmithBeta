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
	};

	struct Block
	{
		glm::vec3	pos;

		bool front	= false;
		bool back	= false;
		bool right	= false;
		bool left	= false;
		bool top	= false;
		bool bottom = false;

		BlockType	type;
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
		Engine::Renderer::Vertex	vertices[g_vertexPerFace];
	};

	struct EnumHash
	{
		template <typename T>
		uint8_t operator()(T t) const { return static_cast<uint8_t>(t); }
	};
}

