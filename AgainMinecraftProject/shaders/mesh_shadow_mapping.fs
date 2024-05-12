#version 410 core

in VS_OUT {
	vec3 fragPosWorld;
    vec3 normal;
	vec3 texCoords;
	vec4 fragPosEyeSpace;
	mat4 view;
} frag_in;

out vec4 o_fragColor;

uniform sampler2DArray u_textureArray;
uniform sampler2DArray u_shadowMap;

uniform vec3 u_lightDir;
uniform float u_farPlane;

layout (std140) uniform LightSpaceMatrices
{
    mat4 u_lightSpaceMatrices[16];
};

uniform float u_cascadePlaneDistances[16];
uniform int u_cascadeCount;
uniform bool u_showCascades;

float g_near = 180.0f;
float g_far = 200.0f;
vec4 g_fogColor = vec4(147.0f/255.0f, 202.0f/255.0f, 237.0f/255.0f, 1.0f);

const vec3 g_debugColors[5] = {
    vec3(0.0f, 1.0f, 0.0f),
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, 0.0f, 1.0f),
    vec3(1.0f, 1.0f, 0.0f),
    vec3(0.0f, 1.0f, 1.0f)
};

float calculateShadow(vec3 fragPosWorldSpace);
vec3 getDebugColor(vec3 fragPosWorldSpace);

void main()
{
	float dist = length(frag_in.fragPosEyeSpace.xyz);
	float fogFactor = clamp((g_far - dist) / (g_far - g_near), 0.0f, 1.0f);

	vec4 textureWithLight = texture(u_textureArray, frag_in.texCoords);
	
    vec3 ambient = 0.35f * textureWithLight.rgb;

    vec3 lightDir = normalize(u_lightDir);
    float diff = max(dot(lightDir, frag_in.normal), 0.0);
    vec3 diffuse = 1.5f * diff * textureWithLight.rgb;

    float shadow = calculateShadow(frag_in.fragPosWorld);

    textureWithLight = 
        vec4((ambient + (1.0f - shadow) * diffuse) * textureWithLight.rgb, textureWithLight.a);

    if (u_showCascades)
    {
        vec4 shadowMapDebugColor = vec4(getDebugColor(frag_in.fragPosWorld), 1.0f);
        textureWithLight *= shadowMapDebugColor;
    }
    //o_fragColor = textureWithLight;
	o_fragColor = mix(g_fogColor, textureWithLight, fogFactor);
	//o_fragColor = texture(u_textureArray, frag_texCoords);
}

float calculateShadow(vec3 fragPosWorldSpace)
{
    vec4 fragPosViewSpace = frag_in.view * vec4(fragPosWorldSpace, 1.0f);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < u_cascadeCount; i++)
    {
        if (depthValue < u_cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = u_cascadeCount;
    }

    vec4 fragPosLightSpace = u_lightSpaceMatrices[layer] * vec4(fragPosWorldSpace, 1.0f);
	// perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5f + 0.5f;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0f)
    {
        return 0.0f;
    }
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(frag_in.normal);
    //vec3 lightDir = normalize(-u_lightDir);
    float bias = max(0.05f * (1.0f - dot(normal, u_lightDir)), 0.03f);
    const float biasModifier = 0.5f;
    if (layer == u_cascadeCount)
    {
        bias *= 1 / (u_farPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (u_cascadePlaneDistances[layer] * biasModifier);
    }
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0f;
    vec2 texelSize = 1.0f / vec2(textureSize(u_shadowMap, 0));
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(
                u_shadowMap, 
                vec3(projCoords.xy + vec2(x, y) * texelSize, 
                layer)
            ).r; 
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0f : 0.0f;        
        }    
    }
    shadow /= 9.0f;
        
    return shadow;
}

vec3 getDebugColor(vec3 fragPosWorldSpace)
{
    vec4 fragPosViewSpace = frag_in.view * vec4(fragPosWorldSpace, 1.0f);
    float depthValue = abs(fragPosViewSpace.z);

    for (int i = 0; i < u_cascadeCount; i++)
    {
        if (depthValue < u_cascadePlaneDistances[i])
        {
            return g_debugColors[i];
        }
    }        
    return g_debugColors[u_cascadeCount];
}
