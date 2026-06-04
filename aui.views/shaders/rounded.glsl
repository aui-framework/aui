float sdRoundedBox(in vec2 p, in vec2 b, in float r) {
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

float rounded(vec2 absolute, vec2 size, vec2 absoluteDerivatives) {
    vec2 fw = max(absoluteDerivatives, 1e-7);
    vec2 halfSize = 1.0 / fw;
    float r = size.x * halfSize.x;
    float d = sdRoundedBox(absolute * halfSize, halfSize, r);
    return smoothstep(0.5, -0.5, d);
}
