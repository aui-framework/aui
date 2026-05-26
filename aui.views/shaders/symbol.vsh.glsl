#version 120
attribute vec2 pos;
attribute vec2 uv;
attribute vec4 color;
varying vec2 vUv;
varying vec4 vColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vUv = uv;
    vColor = color;
}
