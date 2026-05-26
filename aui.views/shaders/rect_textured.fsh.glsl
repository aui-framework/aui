in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;
void main() {
    gl_FragColor = texture2D(albedo, vUv) * vColor;
}
