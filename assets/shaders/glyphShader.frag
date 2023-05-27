#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragTexIndex;
layout(location = 2) in vec4 fragColor;

layout(set = 2, binding = 0) uniform sampler2D vGlobalTextures[];

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor * texture(vGlobalTextures[int(fragTexIndex)], fragTexCoord);
}