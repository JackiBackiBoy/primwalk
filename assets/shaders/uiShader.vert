#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float a_TexIndex;

out vec2 texCoord;
out float v_TexIndex;

uniform mat4 projMat;

void main() {
  gl_Position = projMat * vec4(aPos.xyz, 1.0);
  texCoord = aTexCoord;
  v_TexIndex = a_TexIndex;
}
