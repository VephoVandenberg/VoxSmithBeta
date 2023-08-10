#pragma once

namespace Engine
{
	struct Texture
	{
		unsigned int id;
		int width, height, nrChannels;
	};

	bool initTexture(Texture& texture, const char* path);
	void useTexture(Texture& texture);
}
