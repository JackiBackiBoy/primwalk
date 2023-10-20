#version 450
#extension GL_EXT_nonuniform_qualifier : enable

// layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];
layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];

layout(push_constant) uniform Push {
    mat4 modelMatrix;
	vec3 color;
    uint diffuseTexIndex;
} push;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragTangent;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

const vec3 lightDir = vec3(1.0, 0.5, 0.8);

void main() {
    // Ambient
    float ambient = 0.3;

    // Diffuse
    float diffuse = max(0.0, dot(normalize(fragNormal), normalize(lightDir)));

    vec3 result = (ambient + diffuse) * push.color;
    outColor = texture(vGlobalTextures[int(push.diffuseTexIndex)], fragTexCoord) * vec4(result, 1.0);
}