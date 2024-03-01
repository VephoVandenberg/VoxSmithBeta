#pragma once

#include <map>
#include <vector>

namespace Engine
{
	namespace ShadersAvailable {
		static const char* s_cubeShader = "cube";
		static const char* s_meshShader = "mesh";
		static const char* s_rayShader = "ray";
		static const char* s_outlineShader = "outline";
		static const char* s_meshAndShadow = "mesh_shadow";
		static const char* s_shadowDepth = "shadow_depth";
		static const char* s_cascadedDepth = "shadow_cascaded_depth";
		static const char* s_lightObj = "light_obj";
		static const char* s_debugQuad = "debug_quad";
	}

	using ShaderProgram = std::pair<const char*, const char*>;
	using ShaderProgramExtended = std::vector<const char*>;

	static const std::map<const char*, ShaderProgram> s_shaderPaths = {
		{ShadersAvailable::s_cubeShader,		{"shaders/cube_shader.vs",			"shaders/cube_shader.fs"}},
		{ShadersAvailable::s_meshShader,		{"shaders/mesh_shader.vs",			"shaders/mesh_shader.fs"}},
		{ShadersAvailable::s_rayShader,			{"shaders/ray_shader.vs",			"shaders/ray_shader.fs"}},
		{ShadersAvailable::s_outlineShader,		{"shaders/face_outline_shader.vs",	"shaders/face_outline_shader.fs"}},
		{ShadersAvailable::s_meshAndShadow,		{"shaders/mesh_shadow_mapping.vs",	"shaders/mesh_shadow_mapping.fs"}},
		{ShadersAvailable::s_shadowDepth,		{"shaders/shadow_mapping_depth.vs",	"shaders/shadow_mapping_depth.fs"}},
		{ShadersAvailable::s_lightObj,			{"shaders/light_obj.vs",			"shaders/light_obj.fs"}},
		{ShadersAvailable::s_debugQuad,			{"shaders/debug_quad.vs",			"shaders/debug_quad.fs"}},
	};

	static const std::map<const char*, ShaderProgramExtended> s_extendedShaderPaths = {
		{
			ShadersAvailable::s_cascadedDepth, {	
				"shaders/shadow_cascade_mapping_depth.vs", 
				"shaders/shadow_mapping_depth.gs", 
				"shaders/shadow_mapping_depth.fs"
			},
		}
	};
}
