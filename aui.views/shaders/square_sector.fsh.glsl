in vec2 vUv;
in vec4 vColor;
uniform float begin;
uniform float end;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

void main() {
    vec2 uv = vUv * 2.0 - 1.0;
    float angle = atan(uv.y, uv.x);
    if (angle < 0.0) angle += 6.28318530718;
    if (angle < begin || angle > end) discard;
    gl_FragColor = vColor;
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
