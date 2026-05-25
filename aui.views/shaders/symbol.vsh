uniform {
  mat4 transform
  float uvScale
}

input {
  [0] vec2 pos
  [1] vec2 uv
  [2] vec4 color
}
inter {
  vec2 uv
  vec4 color
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 0, 1)
    inter.uv = input.uv * uniform.uvScale
    inter.color = input.color
}
