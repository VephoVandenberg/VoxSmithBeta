#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DArray u_depthMap;
uniform int u_layer;

void main()
{
    float depthValue = texture(u_depthMap, vec3(TexCoords, u_layer)).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0f); // orthographic
}
