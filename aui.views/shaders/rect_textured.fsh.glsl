#version 120
varying vec2 vUv;
varying vec4 vColor;
uniform sampler2D albedo;
void main() {
    gl_FragColor = texture2D(albedo, vUv) * vColor;
}
