#version 450

struct DirectionLightParams {
    vec3 direction;
    vec3 color; // TODO: Not actually needed at all in shadow mapping
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 viewPosition;
    DirectionLightParams directionLight;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * positionWorld;
}