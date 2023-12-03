#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in int aCoordInd;
layout (location = 2) in float aTexInd;

out vec3 fragTexCoords;

uniform mat4 u_projection;
uniform mat4 u_view;

const vec2 g_texCoords[4] = vec2[](
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f)
);

void main()
{
	gl_Position		= u_projection * u_view * vec4(aPos, 1.0f);
	fragTexCoords	= vec3(g_texCoords[aCoordInd], aTexInd);
}