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
        if (maskUv.x < 0.0 || maskUv.x > 1.0 || maskUv.y < 0.0 || maskUv.y > 1.0) {
            fragColor *= 0.0;
        } else {
            fragColor *= texture(u_mask, maskUv).r;
        }
    }
}
