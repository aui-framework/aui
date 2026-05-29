in vec2 vUv;
in vec4 vColor;
in vec2 vInnerSize;
in vec2 vOuterToInner;
in vec2 vOuterSize;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

float rounded(vec2 absolute, vec2 size) {
    vec2 circleCenter = 1.0 - size;
    
#if defined(GL_ES) && !defined(GL_OES_standard_derivatives)
    vec2 fw = vec2(0.01);
#else
    vec2 fw = fwidth(absolute);
#endif

    vec2 safeFw = max(fw, 1e-7);
    vec2 d_corner = (absolute - circleCenter) / safeFw;
    float radius_pixels = (size.x / safeFw.x + size.y / safeFw.y) * 0.5;
    
    vec2 max_d = max(d_corner, 0.0);
    float dist_pixels = length(max_d) + min(max(d_corner.x, d_corner.y), 0.0) - radius_pixels;
    
    return smoothstep(0.5, -0.5, dist_pixels);
}

void main() {
    vec2 absolute = abs(vUv * 2.0 - 1.0);
    gl_FragColor = vColor * (rounded(absolute, vOuterSize) - rounded(absolute * vOuterToInner, vInnerSize));
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
    if (gl_FragColor.a < 0.001) discard;
}
