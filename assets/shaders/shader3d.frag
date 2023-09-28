#version 450
#extension GL_EXT_nonuniform_qualifier : enable

// layout(set = 1, binding = 0) uniform sampler2D vGlobalTextures[];

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}