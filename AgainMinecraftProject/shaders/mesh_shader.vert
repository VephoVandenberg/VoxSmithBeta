#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoords;

out vec3 fragTexCoords;

uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
	gl_Position		= u_projection * u_view * vec4(aPos, 1.0f);
	fragTexCoords	= aTexCoords;
}