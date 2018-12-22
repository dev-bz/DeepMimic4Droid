#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
uniform sampler2D gBufferTex;
in vec2 TexCoord;

out vec4 outColor;

void main(void) { outColor = texture(gBufferTex, TexCoord.xy); }