in vec2 vUv;
in vec4 vColor;
uniform float divider;
uniform float threshold;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

float dashed(float d) {
    return step(mod(d, divider), threshold);
}
void main() {
    gl_FragColor = vColor * dashed(vUv.x);
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
