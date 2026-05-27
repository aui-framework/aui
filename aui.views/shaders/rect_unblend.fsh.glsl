in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;
void main() {
    vec4 tex = texture2D(albedo, vUv);
    tex.rgb *= tex.a;
    gl_FragColor = tex * vColor;
}
