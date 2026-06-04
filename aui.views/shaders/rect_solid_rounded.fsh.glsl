layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
in vec2 vOuterSize;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

float sdRoundedBox(in vec2 p, in vec2 b, in float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

float rounded(vec2 absolute, vec2 size, vec2 absoluteDerivatives) {
    vec2 fw = max(absoluteDerivatives, 1e-7);
    vec2 halfSize = 1.0 / fw;
    float r = size.x * halfSize.x;
    float d = sdRoundedBox(absolute * halfSize, halfSize, r);
    return smoothstep(0.5, -0.5, d);
}

void main() {
    fragColor = vColor * rounded(abs(vUv * 2.0 - 1.0), vOuterSize, fwidth(vUv) * 2.0);
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        if (maskUv.x < 0.0 || maskUv.x > 1.0 || maskUv.y < 0.0 || maskUv.y > 1.0) {
            fragColor *= 0.0;
        } else {
            fragColor *= texture(u_mask, maskUv).r;
        }
    }
    if (fragColor.a < 0.001) discard;
}
