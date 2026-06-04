layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
in vec2 vOuterSize;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

float rounded(vec2 absolute, vec2 size, vec2 absoluteDerivatives) {
    vec2 circleCenter = 1.0 - size;
    
#if defined(GL_ES) && !defined(GL_OES_standard_derivatives)
    vec2 fw = vec2(0.01);
#else
    vec2 fw = absoluteDerivatives;
#endif

    vec2 safeFw = max(fw, 1e-7);
    vec2 d_corner = (absolute - circleCenter) / safeFw;
    float radius_pixels = (size.x / safeFw.x + size.y / safeFw.y) * 0.5;
    
    vec2 max_d = max(d_corner, 0.0);
    float dist_pixels = length(max_d) + min(max(d_corner.x, d_corner.y), 0.0) - radius_pixels;
    
    return smoothstep(0.5, -0.5, dist_pixels);
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
