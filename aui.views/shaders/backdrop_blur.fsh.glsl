layout(location = 0) out vec4 fragColor;
in vec2 vUv;
in vec4 vColor;

uniform sampler2D albedo;
uniform vec2 pixel_to_uv;
uniform vec2 m2;
uniform float kernel[129];
uniform int uKernelRadius;

void main() {
    vec3 accumulator = vec3(0.0);
    vec2 baseUv = m2 * vUv;

    for (int i = -64; i <= 64; ++i) {
        if (abs(i) > uKernelRadius) {
            continue;
        }
        float f = float(i);
        vec2 uv = pixel_to_uv * f + baseUv;
        uv = clamp(uv, vec2(0.0), m2);
        accumulator += texture(albedo, uv).xyz * kernel[uKernelRadius + i];
    }

    fragColor = vec4(accumulator, texture(albedo, clamp(baseUv, vec2(0.0), m2)).a) * vColor;
}
