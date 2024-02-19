uniform {
  vec4 color1
  vec4 color2
  mat3 matUv
}

vec4 gradient(vec2 uv) {
  vec3 transformedUv = uniform.matUv * vec3(uv, 1)
  vec4 c = mix(uniform.color1, uniform.color2, clamp(transformedUv.x, 0, 1))
  return c
}