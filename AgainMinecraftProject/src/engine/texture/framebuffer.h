#pragma once

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
	void setViewport(const size_t width, const size_t height);

	struct FBuffer
	{
		uint32_t ID;
		uint32_t map;
	};

	void initFBuffer(FBuffer& fBuffer);
	void initFArrayBuffer(FBuffer& fBuffer, uint32_t n, const std::vector<glm::vec3>& cascades);
	void bindFBuffer(FBuffer& fBuffer);
	void unbindFBuffer();
	void setFramebufferViewport();
}
