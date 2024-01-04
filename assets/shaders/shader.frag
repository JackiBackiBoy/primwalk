#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in float fragTexIndex;
layout(location = 2) in vec4 fragColor;
layout(location = 3) flat in uint fragTexCoordIndex;
layout(location = 4) flat in uint fragBorderRadius;
layout(location = 5) in vec2 fragSize;

layout(set = 2, binding = 0) uniform sampler2D vGlobalTextures[];

layout(location = 0) out vec4 outColor;

float roundedBoxSDF(vec2 centerPosition, vec2 size, float radius) {
    return length(max(abs(centerPosition)- size + radius, 0.0)) - radius;
}

const float sqrtTwo = sqrt(2);

void main() {
    // Compute the hypotenuse of radius x radius corner block.
    // This is effectively the radius value used for box SDF
    float radius = sqrtTwo * fragBorderRadius;
    float edgeSoftness = 1.0;

    float fragWidth = fragSize.x;
    float fragHeight = fragSize.y;
    vec2 size = vec2(fragWidth, fragHeight);
    vec2 uvRadius = vec2(radius / fragWidth, radius / fragHeight);

    // Corner positions within the rectangle
    vec2 topLeft = vec2(uvRadius.x, 1.0 - uvRadius.y);
    vec2 topRight = vec2(1.0 - uvRadius.x, 1.0 - uvRadius.y);
    vec2 bottomLeft = vec2(uvRadius.x, uvRadius.y);
    vec2 bottomRight = vec2(1.0 - uvRadius.x, uvRadius.y);
    vec2 uvScreen = vec2(fragTexCoord.x * fragWidth, fragTexCoord.y * fragHeight);

    float distance 		= roundedBoxSDF(uvScreen - (size / 2.0), size / 2.0, radius);
    float smoothedAlpha =  1.0 - smoothstep(0.0, edgeSoftness, distance);

    outColor = fragColor * texture(vGlobalTextures[int(fragTexIndex)], fragTexCoord) * vec4(1.0, 1.0, 1.0, smoothedAlpha);
}