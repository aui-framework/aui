in vec4 vVertex;
in vec4 vColor;
uniform float divider;
uniform float threshold;
float dashed(float d) {
    return step(mod(d, divider), threshold);
}
void main() {
    gl_FragColor = vColor * dashed(vVertex.z);
}
