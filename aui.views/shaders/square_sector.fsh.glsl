#version 120
varying vec2 vUv;
varying vec4 vColor;
uniform float begin;
uniform float end;
void main() {
    vec2 uv = vUv * 2.0 - 1.0;
    float angle = atan(uv.y, uv.x);
    if (angle < 0.0) angle += 6.28318530718;
    if (angle < begin || angle > end) discard;
    gl_FragColor = vColor;
}
