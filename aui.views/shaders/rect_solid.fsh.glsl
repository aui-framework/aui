in vec4 vColor;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

void main() {
    gl_FragColor = vColor;
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
