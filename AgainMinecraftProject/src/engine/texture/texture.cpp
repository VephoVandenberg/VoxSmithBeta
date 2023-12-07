#include <string>
#include <iostream>

#include <glad/glad.h>

#include "stb_loader.h"
#include "texture.h"

using namespace Engine;

bool Engine::initTexture(Texture& texture, const char* path)
{
	unsigned char* data = stbi_load(path, &texture.width, &texture.height, &texture.nrChannels, 0);

	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (std::string(path).find(".jpg") && data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return true;
	}
	
	if (std::string(path).find(".png") && data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
		return true;
	}

	std::cout << "Could not initalize texture" << std::endl;
	exit(EXIT_FAILURE);
	return false;
}

bool Engine::initTextureArray(TextureArray& tArray, const std::vector<const char*>& paths)
{
	glGenTextures(1, &tArray.id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tArray.id);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 64, 64, paths.size());
	for (uint8_t index = 0; index < paths.size(); index++)
	{
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(paths[index], &tArray.width, &tArray.height, &tArray.nrChannels, 0);

		if (!data)
		{
			std::cout << "Could not upload data." << std::endl;
			exit(EXIT_FAILURE);
		}

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			0,
			0,
			index,
			tArray.width,
			tArray.height,
			1,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Engine::useTextureArray(TextureArray& tArray)
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, tArray.id);
}

void Engine::useTexture(Texture& texture)
{
	glBindTexture(GL_TEXTURE_2D, texture.id);
}
