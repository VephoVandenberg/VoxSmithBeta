#version 400 core

in VS_OUT {
	vec3 fragPos;
    vec3 normal;
	vec3 texCoords;
	vec4 fragPosEyeSpace;
	vec4 fragPosLightSpace;
} frag_in;

out vec4 o_fragColor;

uniform sampler2D u_shadowMap;
uniform sampler2DArray u_textureArray;

uniform vec3 u_lightPos;
uniform vec3 u_viewPos;

float g_near = 200.0f;
float g_far = 230.0f;
vec4 g_fogColor = vec4(147.0f/255.0f, 202.0f/255.0f, 237.0f/255.0f, 1.0f);

float calculateShadow(vec4 fragPosLightSpace)
{
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(frag_in.normal);
    vec3 lightDir = normalize(u_lightPos - frag_in.fragPos);
    float bias = max(0.05f * (1.0f - dot(normal, lightDir)), 0.015f);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0f;
    vec2 texelSize = 1.0f / textureSize(u_shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0f : 0.0f;        
        }    
    }
    shadow /= 9.0f;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0f)
        shadow = 0.0f;
        
    return shadow;
}

void main()
{
	float dist = length(frag_in.fragPosEyeSpace.xyz);
	float fogFactor = clamp((g_far - dist) / (g_far - g_near), 0.0f, 1.0f);

	vec4 textureWithLight = texture(u_textureArray, frag_in.texCoords);
	textureWithLight = vec4(textureWithLight.rgb, textureWithLight.a);

	o_fragColor = mix(g_fogColor, textureWithLight, fogFactor);
	//o_fragColor = texture(u_textureArray, frag_texCoords);
}
