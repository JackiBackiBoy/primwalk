#version 450

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout (set = 0, binding = 0) uniform sampler2D positionBuffer;
layout (set = 0, binding = 1) uniform sampler2D normalBuffer;
layout (set = 0, binding = 2) uniform sampler2D diffuseBuffer;
layout (set = 0, binding = 3) uniform sampler2D specularBuffer;

layout (set = 1, binding = 0) uniform UBO {
    vec3 viewPosition;
    PointLightParams pointLights[32];
    uint numLights;
} ubo;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

#define MAX_LIGHTS 32
#define ambientIntensity 0.3

void main() {
    vec3 fragPos = texture(positionBuffer, inUV).rgb;
    vec3 normal = normalize(texture(normalBuffer, inUV).rgb);
    vec3 diffuse = texture(diffuseBuffer, inUV).rgb;

    vec3 diffuseLight = vec3(ambientIntensity);
    vec3 specularLight = vec3(0.0);

    vec3 viewDir = normalize(ubo.viewPosition - fragPos);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLightParams light = ubo.pointLights[i];
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
        specularLight += intensity * blinnTerm;

        diffuseLight += intensity * cosAngIncidence;
    }

    outColor = vec4(diffuse * (diffuseLight + specularLight), 1.0);
}