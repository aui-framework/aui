in vec2 pos;
in vec4 color;
out vec4 vColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vColor = color;
}
