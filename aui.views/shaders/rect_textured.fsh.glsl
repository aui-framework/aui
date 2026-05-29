in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

void main() {
    vec4 tex = texture2D(albedo, vUv);
    tex.rgb *= tex.a;
    gl_FragColor = tex * vColor;
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
