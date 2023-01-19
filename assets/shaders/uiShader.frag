#version 330 core

in vec2 texCoord;
in float v_TexIndex;
in vec3 v_Color;

out vec4 FragColor;

uniform sampler2D u_Textures[3];

void main()
{
  int index = int(v_TexIndex);
  FragColor = vec4(v_Color.xyz, 1.0f) * texture(u_Textures[index], texCoord);
}
