#pragma once

#include <glm/glm.hpp>

namespace Engine
{
	struct Shader
	{
		size_t id;
	};

	void initShader(Shader& shader, const char* vPath, const char* fPath);
	void useShader(Shader& shader);
	
	void setUniformi(Shader& shader, const char* uniform, int value);
	void setUniform3f(Shader& shader, const char* uniform, glm::vec3 vec);
	void setUniform4m(Shader& shader, const char* uniform, glm::mat4 mat);
}
