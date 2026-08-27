layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;
out vec2 vUv;
out vec4 vColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vUv = uv;
    vColor = color;
}
