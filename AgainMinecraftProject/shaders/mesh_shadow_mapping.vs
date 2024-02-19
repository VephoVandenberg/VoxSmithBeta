#version 400 core

layout (location = 0) in uint aData;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_lightSpaceMatrix;
uniform vec3 u_chunkPos;

out VS_OUT {
	vec3 fragPos;
    vec3 normal;
	vec3 texCoords;
	vec4 fragPosEyeSpace;
	vec4 fragPosLightSpace;
} frag_in;

const vec2 g_texCoords[4] = vec2[4](
	vec2(0, 0),
	vec2(1, 0),
	vec2(0, 1),
	vec2(1, 1)
);

const vec3 g_normals[6] = vec3[6](
	vec3( 1,  0,  0), // +x
	vec3(-1,  0,  0), // -x
	vec3( 0,  1,  0), // +y
	vec3( 0, -1,  0), // -y
	vec3( 0,  0,  1), // +z
	vec3( 0,  0, -1)  // -z
);

void main()
{
	vec3 coords = vec3(
		aData & 0x1F,			// x
		(aData >> 5) & 0x1FF,	// y
		(aData >> 14) & 0x1F	// z
	);

	uint coordInd	= (aData >> 19) & 0x3;
	uint texId		= (aData >> 21) & 0xF;
	uint normalId	= (aData >> 25) & 0x3;

	vec4 pos		= vec4(coords + u_chunkPos, 1.0f);
	gl_Position		= u_projection * u_view * pos;
	
	frag_in.fragPosLightSpace	= u_lightSpaceMatrix * pos;
	frag_in.fragPosEyeSpace		= u_view * pos;
	frag_in.texCoords			= vec3(g_texCoords[coordInd], texId);
	frag_in.normal				= transpose(inverse(mat3(mat3(1.0f)))) * g_normals[normalId];
}
