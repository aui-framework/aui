uniform {
  mat4 transform
  float uvScale
}

input {
  [0] vec2 pos
  [1] vec2 uv
}
inter {
  vec2 uv
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 0, 1)
    inter.uv = input.uv * uniform.uvScale
}