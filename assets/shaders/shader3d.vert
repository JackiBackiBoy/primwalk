#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(inPosition, 1.0);

    vec4 newNormal = push.modelMatrix * vec4(inNormal, 0.0);
    fragNormal = newNormal.xyz;
    fragTangent = inTangent;
    fragTexCoord = inTexCoord;
}