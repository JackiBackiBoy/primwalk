#version 450

struct DirectionLightParams {
    vec3 direction;
    vec3 color; // w component is intensity for now
};

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout (set = 0, binding = 0) uniform sampler2D positionBuffer;
layout (set = 0, binding = 1) uniform sampler2D normalBuffer;
layout (set = 0, binding = 2) uniform sampler2D albedoBuffer;
layout (set = 0, binding = 3) uniform sampler2D specularBuffer;

layout (set = 1, binding = 0) uniform UBO {
    vec3 viewPosition;
    DirectionLightParams directionLight;
    PointLightParams pointLights[32];
    uint numLights;
} ubo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

#define MAX_LIGHTS 32
#define ambientIntensity 0.3

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

    // 1. Calculate direction light
    vec3 result = vec3(ambientIntensity) + calcDirLight(ubo.directionLight, normal, viewDir);

    // 2. Calculate point lights
    for (int i = 0; i < ubo.numLights; i++) {
        result += calcPointLight(ubo.pointLights[i], normal, fragPos, viewDir);
    }

    outColor = vec4(albedo * result, 1.0);
}