varying vec4 vVertex;
varying vec4 vColor;
uniform vec2 lower;
uniform vec2 upper;
uniform float sigma;

vec4 erf(vec4 x) {
    vec4 s = sign(x);
    vec4 a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x = x * x;
    return s - s / (x * x);
}

void main() {
    vec4 result = vColor;
    vec2 v = vVertex.xy;
    vec4 query = vec4(v - lower, v - upper);
    vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));
    result.a = result.a * clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0);
    gl_FragColor = result;
}
