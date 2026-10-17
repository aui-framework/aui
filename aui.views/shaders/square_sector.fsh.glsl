layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;
uniform float u_begin;
uniform float u_end;

uniform sampler2D u_mask;
uniform bool u_useMask;
uniform vec2 u_windowSize;
uniform vec4 u_maskRect;

const float PI2 = 6.28318530718;

void main() {
    if (u_end - u_begin >= PI2 - 0.0001) {
        fragColor = vColor;
    } else {
        vec2 uv = vUv * 2.0 - 1.0;
        float angle = atan(uv.x, -uv.y);
        if (angle < 0.0) angle += PI2;
        
        float b = mod(u_begin, PI2);
        if (b < 0.0) b += PI2;
        float e = mod(u_end, PI2);
        if (e < 0.0) e += PI2;
        
        bool inside = false;
        if (b < e) {
            inside = (angle >= b && angle <= e);
        } else {
            inside = (angle >= b || angle <= e);
        }
        
        if (!inside) discard;
        fragColor = vColor;
    }
    
    if (u_useMask) {
        vec2 maskUv = (gl_FragCoord.xy - u_maskRect.xy) / u_maskRect.zw;
        fragColor *= texture(u_mask, maskUv).r;
    }
}
