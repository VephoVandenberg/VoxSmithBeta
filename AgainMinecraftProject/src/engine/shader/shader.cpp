#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "shader_list.h"
#include "shader.h"

using namespace Engine;

void Engine::loadShaders(std::map<const char*, Shader>& shaders)
{
	for (auto& el : s_shaderPaths)
	{
		const auto& sName = el.first;
		const auto& vPath = el.second.first;
		const auto& fPath = el.second.second;

		Shader shader;
		initShader(shader, vPath, fPath);
		shaders.insert({ sName, shader });
	}

	for (auto& el : s_extendedShaderPaths)
	{
		const auto& sName = el.first;
		const auto& vPath = el.second[0];
		const auto& gPath = el.second[1];
		const auto& fPath = el.second[2];

		Shader shader;
 		initShader(shader, vPath, gPath, fPath);
		shaders.insert({ sName, shader });
	}
}

void Engine::initShader(Shader& shader, const char* vPath, const char* fPath)
{
	std::ifstream vFile(vPath);
	std::ifstream fFile(fPath);

	if (!vFile.is_open())
	{
		std::cout << "ERROR::VERTEX_SHADER::FILE_IS_NOT_OPENED:" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!fFile.is_open())
	{
		std::cout << "ERROR::FRAGMENT_SHADER::FILE_IS_NOT_OPENED:" << std::endl;
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
		std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED:" << std::endl;
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
		std::cout << "ERROR::SHADER_PROGRAM::LINK_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	glDeleteShader(vID);
	glDeleteShader(fID);
}

void Engine::initShader(Shader& shader, const char* vPath, const char* gPath, const char* fPath)
{
	std::ifstream vFile(vPath);
	std::ifstream gFile(gPath);
	std::ifstream fFile(fPath);

	if (!vFile.is_open())
	{
		std::cout << "ERROR::VERTEX_SHADER::FILE_IS_NOT_OPENED:" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!gFile.is_open())
	{
		std::cout << "ERROR::GEOMETRY_SHADER::FILE_IS_NOT_OPENED:" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (!fFile.is_open())
	{
		std::cout << "ERROR::FRAGMENT_SHADER::FILE_IS_NOT_OPENED:" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream vStream, gStream, fStream;

	vStream << vFile.rdbuf();
	gStream << gFile.rdbuf();
	fStream << fFile.rdbuf();

	vFile.close();
	gFile.close();
	fFile.close();

	std::string vString = vStream.str();
	std::string gString = gStream.str();
	std::string fString = fStream.str();

	uint32_t vID, gID, fID;
	char infolog[512];
	int32_t success;

	vID = glCreateShader(GL_VERTEX_SHADER);
	const char* vSrc = vString.c_str();
	glShaderSource(vID, 1, &vSrc, nullptr);
	glCompileShader(vID);
	glGetShaderiv(vID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vID, 512, nullptr, infolog);
		std::cout << "ERROR::VERTEX_SHADER::COMPILATION_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	gID = glCreateShader(GL_GEOMETRY_SHADER);
	const char* gSrc = gString.c_str();
	glShaderSource(gID, 1, &gSrc, nullptr);
	glCompileShader(gID);
	glGetShaderiv(gID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(gID, 512, nullptr, infolog);
		std::cout << "ERROR::GEOMETRY_SHADER::COMPILATION_FAILED:" << std::endl;
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
		std::cout << "ERROR::FRAGMENT_SHADER::COMPILATION_FAILED:" << std::endl;
		std::cout << infolog << std::endl;
		exit(EXIT_FAILURE);
	}

	shader.id = glCreateProgram();
	glAttachShader(shader.id, vID);
	glAttachShader(shader.id, gID);
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
	glDeleteShader(gID);
	glDeleteShader(fID);
}

void Engine::useShader(Shader& shader)
{
	glUseProgram(shader.id);
}

int32_t getAndCheckUniformLocation(Shader& shader, const char* uniform)
{
	int location = glGetUniformLocation(shader.id, uniform);
	if (location == -1)
	{
		std::cout << "ERROR::UNIFORM::UNIFORM_NOT_FOUND:" << uniform << std::endl;
	}
	return location;
}

void Engine::setUniformi(Shader& shader, const char* uniform, int32_t value)
{
	useShader(shader);
	glUniform1i(getAndCheckUniformLocation(shader, uniform), value);
}

void Engine::setUniformf(Shader& shader, const char* uniform, float value)
{
	useShader(shader);
	glUniform1f(getAndCheckUniformLocation(shader, uniform), value);
}

void Engine::setUniform3f(Shader& shader, const char* uniform, const glm::vec3& vec)
{
	useShader(shader);
	glUniform3fv(getAndCheckUniformLocation(shader, uniform), 1, &vec[0]);
}

void Engine::setUniform4m(Shader& shader, const char* uniform, const glm::mat4& mat)
{
	useShader(shader);
	glUniformMatrix4fv(getAndCheckUniformLocation(shader, uniform), 1, false, glm::value_ptr(mat));
}

void Engine::setUniformBool(Shader& shader, const char* uniform, bool value)
{
	useShader(shader);
	glUniform1i(getAndCheckUniformLocation(shader, uniform), value);
}
