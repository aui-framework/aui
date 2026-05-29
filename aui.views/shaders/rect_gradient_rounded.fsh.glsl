in vec2 vUv;
in vec4 vColor;
in vec2 vOuterSize;
in vec4 vColor1;
in vec4 vColor2;
uniform mat3 matUv;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;

vec4 gradient(vec2 uv) {
    vec3 transformedUv = matUv * vec3(uv, 1.0);
    vec4 c = mix(vColor1, vColor2, clamp(transformedUv.x, 0.0, 1.0));
    return c;
}

float rounded(vec2 absolute, vec2 size) {
    vec2 circleCenter = 1.0 - size;
    
#if defined(GL_ES) && !defined(GL_OES_standard_derivatives)
    vec2 fw = vec2(0.01);
#else
    vec2 fw = fwidth(absolute);
#endif

    vec2 safeFw = max(fw, 1e-7);
    vec2 d_corner = (absolute - circleCenter) / safeFw;
    float radius_pixels = (size.x / safeFw.x + size.y / safeFw.y) * 0.5;
    
    vec2 max_d = max(d_corner, 0.0);
    float dist_pixels = length(max_d) + min(max(d_corner.x, d_corner.y), 0.0) - radius_pixels;
    
    return smoothstep(0.5, -0.5, dist_pixels);
}

void main() {
    gl_FragColor = vColor * gradient(vUv) * rounded(abs(vUv * 2.0 - 1.0), vOuterSize);
    if (u_useMask) {
        gl_FragColor *= texture2D(u_mask, gl_FragCoord.xy / u_windowSize).r;
    }
    if (gl_FragColor.a < 0.001) discard;
}
