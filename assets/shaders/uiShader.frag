#version 330 core

in vec3 v_Pos;
in vec2 texCoord;
in float v_TexIndex;
in vec4 v_Color;
in float v_Radius;

out vec4 FragColor;

uniform sampler2D u_Textures[16];
uniform vec2 u_Resolution;

float roundedBoxSDF(vec2 centerPosition, vec2 size, float radius) {
  return length(max(abs(centerPosition)- size + radius, 0.0)) - radius;
}

void main()
{
  float radius = v_Radius;
  float edgeSoftness = 1.0;

  vec2 delta = vec2(dFdx(texCoord.x), dFdy(texCoord.y));
  float fragWidth = 1.0 / delta.x;
  float fragHeight = 1.0 / delta.y;
  vec2 size = vec2(fragWidth, fragHeight);
  vec2 uvRadius = vec2(radius / fragWidth, radius / fragHeight);

  // Corner positions within the rectangle
  vec2 topLeft = vec2(uvRadius.x, 1.0 - uvRadius.y);
  vec2 topRight = vec2(1.0 - uvRadius.x, 1.0 - uvRadius.y);
  vec2 bottomLeft = vec2(uvRadius.x, uvRadius.y);
  vec2 bottomRight = vec2(1.0 - uvRadius.x, uvRadius.y);
  vec2 uvScreen = vec2(texCoord.x * fragWidth, texCoord.y * fragHeight);

  float distance 		= roundedBoxSDF(uvScreen - (size / 2.0), size / 2.0, radius);
  float smoothedAlpha =  1.0 - smoothstep(0.0, edgeSoftness, distance);

  // Return the resultant shape.
  int texIndex = int(v_TexIndex);
  vec4 quadColor		= v_Color * texture(u_Textures[texIndex], texCoord) * vec4(1.0, 1.0, 1.0, smoothedAlpha);
  FragColor 			 = quadColor;
}
