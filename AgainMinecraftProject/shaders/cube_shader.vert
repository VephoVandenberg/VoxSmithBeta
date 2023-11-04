#version 400 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 fragTexCoords;

uniform vec3 u_position;
uniform mat4 u_projection;
uniform mat4 u_view;

void main()
{
	gl_Position		= u_projection * u_view * vec4(aPos + u_position, 1.0f);
	fragTexCoords	= aTexCoords;
}