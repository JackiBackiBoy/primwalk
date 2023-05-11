#version 450

struct RenderParams {
    vec2 pos;
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) readonly buffer RenderParamsBuffer {
    RenderParams params[];
} in_render_params;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

float offsets[] = { 0.0, 300.0 };

void main() {
    RenderParams params = in_render_params.params[gl_InstanceIndex];
    gl_Position = ubo.proj * vec4(inPosition.x + params.pos.x, inPosition.y + params.pos.y, 0.0, 1.0);
    fragColor = inColor;
}