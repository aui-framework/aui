#version 300 es
precision highp float;

flat in vec4 vColorStart;
flat in vec4 vColorEnd;
in float vT;

out vec4 fragColor;

void main() {
  float t = clamp(vT * 0.5 + 0.5, 0.0, 1.0);
  fragColor = mix(vColorStart, vColorEnd, t);
}
