uniform sampler2D gradientMap;
uniform mat3 matUv;
vec4 gradient(vec2 uv) {
    vec3 transformedUv = matUv * vec3(uv, 1.0);
    vec4 c = texture(gradientMap, vec2(transformedUv.x, transformedUv.x));
    return c;
}
