#version 450

struct DirectionLightParams {
    vec3 direction;
    vec3 color;
};

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout (set = 0, binding = 0) uniform sampler2D positionBuffer;
layout (set = 0, binding = 1) uniform sampler2D normalBuffer;
layout (set = 0, binding = 2) uniform sampler2D albedoBuffer;
layout (set = 0, binding = 3) uniform sampler2D specularBuffer;
layout (set = 0, binding = 4) uniform sampler2D shadowMap;

layout (set = 1, binding = 0) uniform UBO {
    vec3 viewPosition;
    DirectionLightParams directionLight;
    PointLightParams pointLights[32];
    uint numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 lightSpaceMatrix;
} push;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

#define MAX_LIGHTS 32
#define ambientIntensity 0.3

// Video Settings
#define USE_PCF

float calcShadowPCF(vec3 projCoords, float currentDepth, float bias, int kernel) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;

    return shadow;
}

float calcShadows(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = vec3(projCoords.xy * 0.5 + 0.5, projCoords.z);

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;

    #ifdef USE_PCF
        shadow = calcShadowPCF(projCoords, currentDepth, bias, 3);
    #else
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    #endif

    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }

    return shadow;
}

vec3 calcDirLight(DirectionLightParams light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.direction);

    // 1. Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // 2. Specular lighting
    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    return (diffuse * light.color + specular * light.color);
}

vec3 calcPointLight(PointLightParams light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 directionToLight = light.position - fragPos;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);
    directionToLight = normalize(directionToLight);

    float cosAngIncidence = max(dot(normal, directionToLight), 0.0);
    vec3 intensity = light.color.xyz * light.color.w * attenuation;

    // Specular component
    vec3 halfAngle = normalize(directionToLight + viewDir);
    float blinnTerm = dot(normal, halfAngle);
    blinnTerm = clamp(blinnTerm, 0, 1);
    blinnTerm = pow(blinnTerm, 32.0); // higher values -> sharper highlights

    vec3 diffuseLight = intensity * cosAngIncidence;
    vec3 specularLight = intensity * blinnTerm;

    return (diffuseLight + specularLight);
}

void main() {
    vec3 fragPos = texture(positionBuffer, inUV).rgb;
    vec3 normal = normalize(texture(normalBuffer, inUV).rgb);
    vec3 albedo = texture(albedoBuffer, inUV).rgb;
    vec3 viewDir = normalize(ubo.viewPosition - fragPos);

    vec4 fragPosLightSpace = push.lightSpaceMatrix * vec4(fragPos, 1.0);

    // 1. Calculate direction light
    vec3 result = vec3(ambientIntensity) + calcDirLight(ubo.directionLight, normal, viewDir);

    // 2. Calculate point lights
    for (int i = 0; i < ubo.numLights; i++) {
        result += calcPointLight(ubo.pointLights[i], normal, fragPos, viewDir);
    }

    // 3. Shadows
    float shadow = calcShadows(fragPosLightSpace, normal, normalize(ubo.directionLight.direction));

    outColor = vec4((1.0 + ambientIntensity - shadow) * albedo * result, 1.0);
}