#version 400 core

layout (location = 0) in uint aData;

uniform mat4 u_lightSpaceMatrix;

uniform vec3 u_chunkPos;

void main()
{
	vec3 pos = vec3(
		aData & 0x1F,			// x
		(aData >> 5) & 0x1FF,	// y
		(aData >> 14) & 0x1F	// z
	);

    gl_Position = u_lightSpaceMatrix * vec4(u_chunkPos + pos, 1.0);
}
