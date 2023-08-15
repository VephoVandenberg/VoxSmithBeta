#pragma once

namespace Engine
{
	struct Texture
	{
		uint32_t id;
		int32_t width, height, nrChannels;
	};

	bool initTexture(Texture& texture, const char* path);
	void useTexture(Texture& texture);
}
