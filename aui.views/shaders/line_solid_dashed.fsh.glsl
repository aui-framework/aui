layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
uniform float divider;
uniform float threshold;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

float dashed(float d) {
    return step(mod(d, divider), threshold);
}
void main() {
    fragColor = vColor * dashed(vUv.x);
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        fragColor *= texture(u_mask, maskUv).r;
    }
}
