#version 450

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    vec3 color;
    uint diffuseTexIndex;
    uint normalMapIndex;
} push;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBiTangent;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out mat3 fragTBN;

void main() {
    vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * positionWorld;

    fragTexCoord = inTexCoord;
    fragPosWorld = positionWorld.xyz;

    // Tangent space calculations
    vec3 T = normalize(vec3(push.modelMatrix * vec4(inTangent, 0.0)));
    vec3 B = normalize(vec3(push.modelMatrix * vec4(inBiTangent, 0.0)));
    vec3 N = normalize(vec3(push.modelMatrix * vec4(inNormal, 0.0)));
    fragTBN = mat3(T, B, N);
}