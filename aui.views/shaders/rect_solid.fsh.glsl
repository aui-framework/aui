layout(location = 0) out vec4 fragColor;
in vec4 vColor;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

void main() {
    fragColor = vColor;
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        fragColor *= texture(u_mask, maskUv).r;
    }
}
