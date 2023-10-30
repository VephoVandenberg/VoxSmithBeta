#version 400 core

in	vec3 fragTexCoords;
out vec4 o_fragColor;

uniform sampler2DArray u_textureArray;

void main()
{
	o_fragColor = texture(u_textureArray, fragTexCoords);
}