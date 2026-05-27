in vec2 vUv;
in vec4 vColor;
in vec2 vInnerSize;
in vec2 vOuterToInner;
in vec2 vOuterSize;

float rounded(vec2 absolute, vec2 size) {
    vec2 circleCenter = 1.0 - size;
    vec2 rectangleShape = step(absolute, circleCenter);
    vec2 circle = (absolute - circleCenter) / (size);
    float circles = step(circle.x * circle.x + circle.y * circle.y, 1.0);
    vec2 rectCut = step(vec2(0.0), vec2(1.0) - absolute);
    return clamp(rectangleShape.x + rectangleShape.y + circles, 0.0, 1.0) * rectCut.x * rectCut.y;
}

void main() {
    vec2 absolute = abs(vUv * 2.0 - 1.0);
    gl_FragColor = vColor * (rounded(absolute, vOuterSize) - rounded(absolute * vOuterToInner, vInnerSize));
    if (gl_FragColor.a < 0.001) discard;
}
