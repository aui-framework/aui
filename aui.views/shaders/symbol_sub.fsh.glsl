in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

void main() {
    vec4 s = texture2D(albedo, vUv);
    vec4 c = vColor;
    gl_FragColor = vec4(c.xyz * s.xyz, (s.x + s.y + s.z) / 3.0 * c.a);
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
