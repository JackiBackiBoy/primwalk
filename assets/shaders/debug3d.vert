#version 450

struct RenderParams {
    vec3 p1;
    vec3 p2;
    vec3 color;
};

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

layout(set = 1, binding = 0) readonly buffer RenderParamsBuffer {
    RenderParams params[];
} in_render_params;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
} push;

layout(location = 0) out vec3 fragColor;

const vec3 positions[2] = vec3[](
    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 1.0, 1.0)
);

void main() {
    RenderParams params = in_render_params.params[gl_InstanceIndex];

    float lengthX = params.p2.x - params.p1.x;
    float lengthY = params.p2.y - params.p1.y;
    float lengthZ = params.p2.z - params.p1.z;

    vec3 position = positions[gl_VertexIndex];
    gl_Position = ubo.proj * ubo.view * push.modelMatrix * vec4(position.x * lengthX + params.p1.x,
                                                                position.y * lengthY + params.p1.y,
                                                                position.z * lengthZ + params.p1.z,
                                                                1.0);

    fragColor = params.color;
}