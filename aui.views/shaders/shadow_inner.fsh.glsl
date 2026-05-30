in vec4 vVertex;
in vec2 vUv;
in vec4 vColor;
uniform vec2 lower;
uniform vec2 upper;
uniform float sigma;
uniform vec2 outerSize;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

vec4 erf(vec4 x) {
    vec4 s = sign(x);
    vec4 a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x = x * x;
    return s - s / (x * x);
}

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
    vec2 v = vVertex.xy;
    vec4 query = vec4(v - lower, v - upper);
    vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));
    float factor = (1.0 - clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0)) * rounded(abs(vUv * 2.0 - 1.0), outerSize, fwidth(vUv) * 2.0);
    gl_FragColor = vColor * factor;
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        if (maskUv.x < 0.0 || maskUv.x > 1.0 || maskUv.y < 0.0 || maskUv.y > 1.0) {
            gl_FragColor *= 0.0;
        } else {
            gl_FragColor *= texture2D(u_mask, maskUv).r;
        }
    }
}
