layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
uniform float begin;
uniform float end;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

void main() {
    vec2 uv = vUv * 2.0 - 1.0;
    float angle = atan(uv.y, uv.x);
    if (angle < 0.0) angle += 6.28318530718;
    if (angle < begin || angle > end) discard;
    fragColor = vColor;
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        if (maskUv.x < 0.0 || maskUv.x > 1.0 || maskUv.y < 0.0 || maskUv.y > 1.0) {
            fragColor *= 0.0;
        } else {
            fragColor *= texture(u_mask, maskUv).r;
        }
    }
}
