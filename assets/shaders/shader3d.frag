#version 450
#extension GL_EXT_nonuniform_qualifier : enable

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

layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
	vec3 color;
    uint diffuseTexIndex;
    uint normalMapIndex;
} push;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

const float ambient = 0.1;

void main() {
    vec3 diffuseLight = vec3(ambient);
    vec3 specularLight = vec3(0.0);
    //vec3 surfaceNormal = normalize(fragNormalWorld);
    vec3 surfaceNormal = texture(vGlobalTextures[int(push.normalMapIndex)], fragTexCoord).rgb;
    surfaceNormal = surfaceNormal * 2.0 - 1.0;
    surfaceNormal = normalize(fragTBN * surfaceNormal);

    vec3 viewDir = normalize(ubo.viewPosition - fragPosWorld);

    // Point light diffuse contribution
    for (int i = 0; i < ubo.numLights; i++) {
        PointLightParams light = ubo.pointLights[i];
        vec3 directionToLight = light.position - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0.0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        // Specular component
        vec3 halfAngle = normalize(directionToLight + viewDir);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 32.0); // higher values -> sharper highlights
        specularLight += intensity * blinnTerm;

        diffuseLight += intensity * cosAngIncidence;
    }

    vec4 diffuseMapColor = texture(vGlobalTextures[int(push.diffuseTexIndex)], fragTexCoord);

    if (diffuseMapColor.a == 0.0) {
        discard;
    }

    outColor = diffuseMapColor * vec4(diffuseLight + specularLight, 1.0);
}