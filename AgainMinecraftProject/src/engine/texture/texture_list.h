#pragma once

#include <vector>

namespace Engine
{
	static const std::vector<const char*> s_tPaths = {
		// Paths to textures		//	Indicies in Texture array
		"textures/grass.png",		//	0
		"textures/dirt_grass.png",	//	1
		"textures/dirt.png",		//	2
		"textures/stone.png",		//	3
		"textures/sand.png",		//  4
		"textures/snow.png",		//  5
		"textures/water.png",		//  6
		//"textures/leaf.png",		//  7
	};

	enum class TextureId : uint8_t
	{
		GRASS = 0,
		DIRT_GRASS,
		DIRT,
		STONE,
		SAND,
		SNOW,
		WATER
	};
}