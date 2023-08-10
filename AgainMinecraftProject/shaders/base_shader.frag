#version 400 core

in vec2 fragTexCoords;
out vec4 oFragColor;

uniform sampler2D uTexture;

void main()
{
	oFragColor = texture(uTexture, fragTexCoords);
}