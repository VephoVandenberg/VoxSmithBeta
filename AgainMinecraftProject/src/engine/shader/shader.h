#pragma once

namespace Engine
{
	struct Shader
	{
		size_t id;
	};

	void initShader(Shader& shader, const char* vPath, const char* fPath);
	void useShader(Shader& shader);
}