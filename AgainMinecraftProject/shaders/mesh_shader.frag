#version 400 core

in vec3 frag_texCoords;
in vec4 frag_eyeCoords;
in float frag_ambientLight;

out vec4 o_fragColor;

uniform sampler2DArray u_textureArray;

float g_near = 0.1f;
float g_far = 225.0f;
vec4 g_fogColor = vec4(147.0f/255.0f, 202.0f/255.0f, 237.0f/255.0f, 1.0f);

void main()
{
	float dist = length(frag_eyeCoords.xyz);
	float fogFactor = clamp((g_far - dist) / (g_far - g_near), 0.0f, 1.0f);

	o_fragColor = mix(g_fogColor, frag_ambientLight * texture(u_textureArray, frag_texCoords), fogFactor);
	// o_fragColor = frag_ambientLight * texture(u_textureArray, frag_texCoords);
}