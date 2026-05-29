in vec4 vVertex;
in vec4 vColor;
uniform vec2 lower;
uniform vec2 upper;
uniform float sigma;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

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
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
}
