vec4 erf(vec4 x) {
    vec4 s = sign(x);
    vec4 a = abs(x);
    x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
    x = x * x;
    return s - s / (x * x);
}
