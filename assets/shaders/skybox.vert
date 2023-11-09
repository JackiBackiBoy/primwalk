#version 450

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 viewPosition;
    PointLightParams pointLights[32];
    uint numLights;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint texIndex;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosWorld;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * positionWorld;
    
    fragPosWorld = positionWorld.xyz;
}