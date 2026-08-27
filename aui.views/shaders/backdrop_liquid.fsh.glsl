layout(location = 0) out vec4 fragColor;
in vec2 vUv;

uniform mat4 m2;
uniform vec2 uvScale;
uniform sampler2D albedo;
uniform sampler2D uvmap;

void main() {
    vec4 uvmapSample = texture(uvmap, vUv);
    vec2 baseUv = (vec2(m2 * vec4(mix(vec2(-1.0), vec2(2.0), uvmapSample.xy), 0.0, 1.0)) + vec2(1.0)) * vec2(0.5);
    baseUv = clamp(baseUv, vec2(0.0), vec2(1.0));
    baseUv *= uvScale;
    vec3 accumulator = texture(albedo, baseUv).xyz;
    fragColor = vec4(accumulator, uvmapSample.a);
}
