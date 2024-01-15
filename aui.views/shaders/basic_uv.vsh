uniform {
  mat4 transform
}

input {
  [0] vec4 pos
  [1] vec2 uv
}
inter {
  vec4 vertex
  vec2 uv
}

entry {
    sl_position = uniform.transform * vec4(input.pos.xy, 0, 1)
    inter.vertex = input.pos
    inter.uv = input.uv
}