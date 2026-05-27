in vec2 vUv;
in vec4 vColor;
uniform vec2 outerSize;
uniform vec4 color1;
uniform vec4 color2;
uniform mat3 matUv;

vec4 gradient(vec2 uv) {
    vec3 transformedUv = matUv * vec3(uv, 1.0);
    vec4 c = mix(color1, color2, clamp(transformedUv.x, 0.0, 1.0));
    return c;
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
    gl_FragColor = vColor * gradient(vUv) * rounded(abs(vUv * 2.0 - 1.0), outerSize);
    if (gl_FragColor.a < 0.001) discard;
}
