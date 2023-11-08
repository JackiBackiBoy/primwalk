#version 450

struct RenderParams {
    vec3 pos;
    vec3 color;
};

layout(set = 1, binding = 0) readonly buffer RenderParamsBuffer {
    RenderParams params[];
} in_render_params;

layout(push_constant) uniform Push {
    mat4 proj;
} push;

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}