#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct PointLightParams {
    vec3 position;
    vec4 color; // w component is intensity
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
	vec3 color;
    uint diffuseTexIndex;
    uint normalMapIndex;
} push;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in mat3 fragTBN;

layout(location = 0) out vec4 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outDiffuse;
layout(location = 3) out vec4 outSpecular;

void main() {
    // Position buffer
    outPosition = vec4(fragPosWorld, 1.0);
    
    // Normal buffer
    vec3 surfaceNormal = texture(vGlobalTextures[int(push.normalMapIndex)], fragTexCoord).rgb;
    surfaceNormal = surfaceNormal * 2.0 - 1.0;
    surfaceNormal = normalize(fragTBN * surfaceNormal);
    outNormal = vec4(surfaceNormal, 1.0);

    // Diffuse buffer
    outDiffuse = texture(vGlobalTextures[int(push.diffuseTexIndex)], fragTexCoord);

    // Specular buffer
    outSpecular = vec4(1.0);
}