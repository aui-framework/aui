in vec2 pos;
in vec2 uv;
in vec4 color;
out vec2 vUv;
out vec4 vColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vUv = uv;
    vColor = color;
}
