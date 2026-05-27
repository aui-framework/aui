in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;
void main() {
    vec4 s = texture2D(albedo, vUv);
    gl_FragColor = vColor * s.x;
}
