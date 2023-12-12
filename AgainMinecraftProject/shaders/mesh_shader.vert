#version 400 core

layout (location = 0) in uint aData;

out vec3 fragTexCoords;

uniform mat4 u_projection;
uniform mat4 u_view;

uniform vec3 u_chunkPos;

const vec2 g_texCoords[4] = vec2[4](
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f)
);

void main()
{
	uint x = aData & 0x1F;
	uint y = (aData >> 5) & 0x1FF;
	uint z = (aData >> 14) & 0x1F;
	
	uint coordInd = (aData >> 19) & 0x3;
	uint texId = (aData >> 21) & 0xF;

	gl_Position		= u_projection * u_view * vec4(vec3(x, y, z) + u_chunkPos, 1.0f);
	fragTexCoords	= vec3(g_texCoords[coordInd], texId);
}