#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
in vec4 VertColor;
out vec4 outColor;
void main() { outColor = VertColor; }