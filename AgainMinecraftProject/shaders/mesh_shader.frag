#version 400 core

in	vec3 fragTexCoords;
in float in_ambientLight;

out vec4 o_fragColor;

uniform sampler2DArray u_textureArray;

void main()
{
	o_fragColor = in_ambientLight * texture(u_textureArray, fragTexCoords);
}