uniform {
  2D gradientMap
  mat3 matUv
}

vec4 gradient(vec2 uv) {
  vec3 transformedUv = uniform.matUv * vec3(uv, 1)
  vec4 c = uniform.gradientMap[vec2(transformedUv.x, transformedUv.x)]
  return c
}