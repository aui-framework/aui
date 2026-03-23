#version 300 es

precision highp float;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color_start;
layout(location = 2) in vec4 color_end;
layout(location = 3) in float rotation;

flat out vec4 vColorStart;
flat out vec4 vColorEnd;
out float vT;

void main() {
  gl_Position = pos;

  vec2 dir = vec2(cos(rotation), sin(rotation));

  vT = dot(pos.xy, dir);

  vColorStart = color_start;
  vColorEnd = color_end;
}
