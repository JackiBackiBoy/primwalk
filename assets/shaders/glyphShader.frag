#version 330 core

in vec3 v_Pos;
in vec2 texCoord;
in float v_TexIndex;
in vec3 v_Color;

out vec4 FragColor;

uniform sampler2D u_Textures[16];

void main()
{
  int index = int(v_TexIndex);
  FragColor = vec4(v_Color.xyz, 1.0) * vec4(1.0, 1.0, 1.0, texture(u_Textures[index], texCoord).r);
}
