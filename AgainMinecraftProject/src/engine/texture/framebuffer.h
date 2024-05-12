#pragma once

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

namespace Engine
{
	static constexpr float g_shadowResolution = 4 * 1024.0f;

	void setViewport(const size_t width, const size_t height);

	struct FBuffer
	{
		uint32_t id;
		uint32_t map;
	};

	void initFBuffer(FBuffer& fBuffer);
	void initFArrayBuffer(FBuffer& fBuffer, const std::vector<float>& cascades);
	void bindFBuffer(FBuffer& fBuffer);
	void useFArray(FBuffer& buffer);
	void unbindFBuffer();
	void setFramebufferViewport();
	void cullFront();
	void cullBack();
}
