float rounded(vec2 absolute, vec2 size) {
    vec2 circleCenter = 1.0 - size;
    vec2 rectangleShape = step(absolute, circleCenter)

    vec2 circle = (absolute - circleCenter) / (size);
    float circles = step(circle.x * circle.x + circle.y * circle.y, 1.0)

    vec2 rectCut = step(vec2(0), vec2(1.0) - absolute)

    return clamp(rectangleShape.x + rectangleShape.y + circles, 0, 1) * rectCut.x * rectCut.y
}
