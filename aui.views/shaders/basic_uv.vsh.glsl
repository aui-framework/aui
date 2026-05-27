in vec2 pos;
in vec2 uv;
in vec4 color;
in vec2 outerSize;
in vec2 innerSize;
in vec2 outerToInner;
in vec4 color1;
in vec4 color2;

out vec4 vVertex;
out vec2 vUv;
out vec4 vColor;
out vec2 vOuterSize;
out vec2 vInnerSize;
out vec2 vOuterToInner;
out vec4 vColor1;
out vec4 vColor2;

uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(pos, 0.0, 1.0);
    vVertex = vec4(pos, 0.0, 1.0);
    vUv = uv;
    vColor = color;
    vOuterSize = outerSize;
    vInnerSize = innerSize;
    vOuterToInner = outerToInner;
    vColor1 = color1;
    vColor2 = color2;
}
