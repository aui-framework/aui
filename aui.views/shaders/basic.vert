#version 300 es

precision highp float;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;
flat out vec4 vertexColor;

void main() {
    gl_Position = vec4(pos.xyz, 1);
}
