#version 410 core

layout (location = 0) in uint aData;

uniform vec3 u_chunkPos;

void main()
{
	vec3 pos = u_chunkPos + vec3(
		aData & 0x1F,			// x
		(aData >> 5) & 0x1FF,	// y
		(aData >> 14) & 0x1F	// z
	);

    gl_Position = vec4(pos, 1.0);
}
