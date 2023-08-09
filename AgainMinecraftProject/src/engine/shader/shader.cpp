#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>

#include "shader.h"

using namespace Engine;

void Engine::initShader(Shader& shader, const char* vPath, const char* fPath)
{
	std::ifstream vFile(vPath);
	std::ifstream fFile(fPath);

	if (!vFile.is_open())
	{
		std::cout << "Could not open vertex path" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!fFile.is_open())
	{
		std::cout << "Could not open fragment file" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream vStream, fStream;

	vStream << vFile.rdbuf();
	fStream << fFile.rdbuf();

	vFile.close();
	fFile.close();

	std::string vString = vStream.str();
	std::string fString = fStream.str();

	unsigned int vID, fID;
	char infolog[512];
	int success;

	vID = glCreateShader(GL_VERTEX_SHADER);
	const char* vSrc = vString.c_str();
	glShaderSource(vID, 1, &vSrc, nullptr);
	glCompileShader(vID);
	glGetShaderiv(vID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vID, 512, NULL, infolog);
		std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	fID = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fSrc = fString.c_str();
	glShaderSource(fID, 1, &fSrc, nullptr);
	glCompileShader(fID);
	glGetShaderiv(fID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fID, 512, NULL, infolog);
		std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	shader.id = glCreateProgram();
	glAttachShader(shader.id, vID);
	glAttachShader(shader.id, fID);
	glLinkProgram(shader.id);
	glGetShaderiv(shader.id, GL_LINK_STATUS, &success);
	if (!success)
	{
		std::cout << "ERROR::SHADER_LINK::LINK_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	glDeleteShader(vID);
	glDeleteShader(fID);
}

void Engine::useShader(Shader& shader)
{
	glUseProgram(shader.id);
}
