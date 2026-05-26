in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;
void main() {
    vec4 s = texture2D(albedo, vUv);
    vec4 c = vColor;
    gl_FragColor = vec4(c.xyz * s.xyz * c.a, (s.x + s.y + s.z) / 3.0 * c.a);
}
