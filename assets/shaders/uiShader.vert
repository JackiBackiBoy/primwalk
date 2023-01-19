#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float a_TexIndex;
layout (location = 3) in vec3 a_Color;

out vec2 texCoord;
out float v_TexIndex;
out vec3 v_Color;

uniform mat4 projMat;

void main() {
  gl_Position = projMat * vec4(aPos.xyz, 1.0);
  texCoord = aTexCoord;
  v_TexIndex = a_TexIndex;
  v_Color = a_Color;
}
