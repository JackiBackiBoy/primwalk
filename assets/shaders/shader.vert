#version 450

struct RenderParams {
    vec2 pos;
    vec2 size;
    vec4 color;
    uint texIndex;
    uint borderRadius;
    vec2 texCoords[4];
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(set = 1, binding = 0) readonly buffer RenderParamsBuffer {
    RenderParams params[];
} in_render_params;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inTexCoordIndex;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out float fragTexIndex;
layout(location = 2) out vec4 fragColor;
layout(location = 3) out uint fragTexCoordIndex;
layout(location = 4) out uint fragBorderRadius;

void main() {
    RenderParams params = in_render_params.params[gl_InstanceIndex];
    gl_Position = ubo.proj * vec4(inPosition.x * params.size.x + params.pos.x, inPosition.y * params.size.y + params.pos.y, 0.0, 1.0);
    fragTexCoord = params.texCoords[inTexCoordIndex];
    fragTexIndex = float(params.texIndex);
    fragTexCoordIndex = inTexCoordIndex;
    fragColor = params.color;
    fragBorderRadius = params.borderRadius;
}