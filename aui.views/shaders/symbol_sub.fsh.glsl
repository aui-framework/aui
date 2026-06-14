layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
uniform sampler2D albedo;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

void main() {
    vec4 s = texture(albedo, vUv);
    vec4 c = vColor;
    fragColor = vec4(c.xyz * s.xyz, (s.x + s.y + s.z) / 3.0 * c.a);
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        fragColor *= texture(u_mask, maskUv).r;
    }
}
