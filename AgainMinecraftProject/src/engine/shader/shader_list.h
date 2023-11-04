#pragma once

#include <map>

namespace Engine
{
	static const char* s_cubeShader		= "cube";
	static const char* s_meshShader		= "mesh";
	static const char* s_rayShader		= "ray";
	static const char* s_outlineShader	= "outline";
	
	using ShaderProgram = std::pair<const char*, const char*>;

	static const std::map<const char*, ShaderProgram> s_shaderPaths = {
		{s_cubeShader,		{"shaders/cube_shader.vert",			"shaders/cube_shader.frag"}},
		{s_meshShader,		{"shaders/mesh_shader.vert",			"shaders/mesh_shader.frag"}},
		{s_rayShader,		{"shaders/ray_shader.vert",				"shaders/ray_shader.frag"}},
		{s_outlineShader,	{"shaders/face_outline_shader.vert",	"shaders/face_outline_shader.frag"}}
	};
}
