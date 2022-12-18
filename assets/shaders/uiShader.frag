#version 330 core

in vec2 texCoord;
in float v_TexIndex;

out vec4 FragColor;

uniform sampler2D u_Textures[2];

void main()
{
  int index = int(v_TexIndex);
  FragColor = texture(u_Textures[index], texCoord);
}
