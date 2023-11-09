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

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    uint texIndex;
} push;

layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];

layout(location = 0) in vec3 fragPosWorld;

layout(location = 0) out vec4 outColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), -asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(fragPosWorld));
    vec3 texColor = texture(vGlobalTextures[int(push.texIndex)], uv).rgb;

    outColor = vec4(texColor, 1.0);
}