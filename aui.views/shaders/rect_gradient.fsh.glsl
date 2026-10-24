layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
uniform vec4 color1;
uniform vec4 color2;
uniform mat3 matUv;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

vec4 gradient(vec2 uv) {
    vec3 transformedUv = matUv * vec3(uv, 1.0);
    vec4 c = mix(color1, color2, clamp(transformedUv.x, 0.0, 1.0));
    return c;
}
void main() {
    fragColor = vColor * gradient(vUv);
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        fragColor *= texture(u_mask, maskUv).r;
    }
}
