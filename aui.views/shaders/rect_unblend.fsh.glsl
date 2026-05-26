varying vec2 vUv;
varying vec4 vColor;
uniform sampler2D albedo;
void main() {
    vec4 unblend = texture2D(albedo, vUv);
    if (unblend.a > 0.0) {
        unblend.xyz = unblend.xyz / unblend.a;
    }
    gl_FragColor = vColor * unblend;
}
