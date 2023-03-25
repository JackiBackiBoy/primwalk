#version 330 core

in vec3 v_Pos;
in vec2 texCoord;
in float v_TexIndex;
in vec4 v_Color;

out vec4 FragColor;

uniform sampler2D u_Textures[8];

void main()
{
  int index = int(v_TexIndex);
  FragColor = v_Color * texture(u_Textures[index], texCoord);
}
