#pragma once

#include <vector>

namespace Engine
{
	struct Texture
	{
		uint32_t id;
		int32_t width, height, nrChannels;
	};

	using TextureArray = Texture;

	bool initTexture(Texture& texture, const char* path);
	bool initTextureArray(TextureArray& texture, const std::vector<const char*>& paths);

	void useTexture(Texture& texture);
	void useTextureArray(TextureArray& array);
}
