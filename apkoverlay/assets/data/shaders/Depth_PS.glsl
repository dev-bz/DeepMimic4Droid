#version 300 es
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
in vec3 ViewPos;
out vec4 outColor;

void main() { outColor = vec4(/*1.f*/-ViewPos.z, 1.f, 1.f, 1.f); }