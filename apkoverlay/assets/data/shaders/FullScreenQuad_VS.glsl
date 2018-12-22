#version 300 es

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

out vec3 Normal;
out vec2 TexCoord;

void main(void) {
  gl_Position = vec4(inPosition.xy, 0.0, 1.0);
  Normal = inNormal;
  TexCoord = inTexCoord;
}