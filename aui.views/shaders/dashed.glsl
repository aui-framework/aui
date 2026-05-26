uniform float divider;
uniform float threshold;
float dashed(float d) {
    return step(mod(d, divider), threshold);
}
