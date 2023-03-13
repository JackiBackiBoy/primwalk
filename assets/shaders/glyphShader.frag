#version 330 core

in vec3 v_Pos;
in vec2 texCoord;
in float v_TexIndex;
in vec3 v_Color;

out vec4 FragColor;

uniform sampler2D u_Textures[16];

float screenPxRange() {
  int index = int(v_TexIndex);
    float pxRange = 2.5;
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_Textures[index], 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);

    return 2.0;
}

float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}

void main()
{
  int index = int(v_TexIndex);

  vec3 msd = texture(u_Textures[index], texCoord).rgb;
  float sd = median(msd.r, msd.g, msd.b);
  float screenPxDistance  = screenPxRange() * (sd - 0.5);
  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
  
  FragColor = vec4(v_Color, opacity);
}
