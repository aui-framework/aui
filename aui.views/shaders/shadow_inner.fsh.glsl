varying vec4 vVertex;
varying vec2 vUv;
varying vec4 vColor;
uniform vec2 lower;
uniform vec2 upper;
uniform float sigma;
uniform vec2 outerSize;

vec4 erf(vec4 x) {
    vec4 s = sign(x);
    vec4 a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x = x * x;
    return s - s / (x * x);
}

float rounded(vec2 absolute, vec2 size) {
    vec2 circleCenter = 1.0 - size;
    vec2 rectangleShape = step(absolute, circleCenter);
    vec2 circle = (absolute - circleCenter) / (size);
    float circles = step(circle.x * circle.x + circle.y * circle.y, 1.0);
    vec2 rectCut = step(vec2(0.0), vec2(1.0) - absolute);
    return clamp(rectangleShape.x + rectangleShape.y + circles, 0.0, 1.0) * rectCut.x * rectCut.y;
}

void main() {
    vec4 result = vColor;
    vec2 v = vVertex.xy;
    vec4 query = vec4(v - lower, v - upper);
    vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / sigma));
    result.a = result.a * (1.0 - clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0));
    result.a = result.a * rounded(abs(vUv * 2.0 - 1.0), outerSize);
    gl_FragColor = result;
}
