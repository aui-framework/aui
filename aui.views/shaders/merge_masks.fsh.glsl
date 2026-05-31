uniform sampler2D u_mask1;
uniform sampler2D u_mask2;
uniform vec4 u_mask1Rect;
uniform vec4 u_mask2Rect;
uniform vec4 u_destRect;
in vec2 vUv;
void main() {
    vec2 windowFragCoord = gl_FragCoord.xy + u_destRect.xy;
    vec2 mask1Uv = (windowFragCoord - u_mask1Rect.xy) / u_mask1Rect.zw;
    float m1 = 0.0;
    if (mask1Uv.x >= 0.0 && mask1Uv.x <= 1.0 && mask1Uv.y >= 0.0 && mask1Uv.y <= 1.0) {
        m1 = texture2D(u_mask1, mask1Uv).r;
    }
    vec2 mask2Uv = (windowFragCoord - u_mask2Rect.xy) / u_mask2Rect.zw;
    float m2 = 0.0;
    if (mask2Uv.x >= 0.0 && mask2Uv.x <= 1.0 && mask2Uv.y >= 0.0 && mask2Uv.y <= 1.0) {
        m2 = texture2D(u_mask2, mask2Uv).r;
    }
    gl_FragColor = vec4(m1 * m2, 0.0, 0.0, 1.0);
}
