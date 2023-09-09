uniform {
  2D gradientMap
}

vec4 gradient(vec2 uv) {
  return uniform.gradientMap[uv.yx]
}