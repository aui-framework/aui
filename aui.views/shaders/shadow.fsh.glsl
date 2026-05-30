in vec4 vVertex;
in vec4 vColor;
uniform vec2 lower;
uniform vec2 upper;
uniform float sigma;

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

void main() {
    vec2 v = vVertex.xy;
    vec4 query = vec4(v - lower, v - upper);
    vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));
    float shadowFactor = clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0);
    gl_FragColor = vColor * shadowFactor;
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        if (maskUv.x < 0.0 || maskUv.x > 1.0 || maskUv.y < 0.0 || maskUv.y > 1.0) {
            gl_FragColor *= 0.0;
        } else {
            gl_FragColor *= texture2D(u_mask, maskUv).r;
        }
    }
}
