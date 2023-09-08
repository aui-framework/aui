uniform {
    vec2 outerSize;
}

float rounded(vec2 uv) {
    vec2 tmp = abs(uv * 2 - 1)
    vec2 circleCenter = 1.0 - uniform.outerSize;
    vec2 rectangleShape = step(tmp, circleCenter)

    vec2 circle = (tmp - circleCenter) / (uniform.outerSize);
    float circles = step(circle.x * circle.x + circle.y * circle.y, 1.0000001)
    return clamp(rectangleShape.x + rectangleShape.y + circles, 0, 1)
}