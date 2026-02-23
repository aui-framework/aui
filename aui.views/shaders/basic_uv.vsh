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
    vec4 position = uniform.transform * vec4(input.pos.xyz, 1)
    position.z = input.pos.z
    inter.vertex = input.pos
    inter.uv = input.uv
    sl_position = position
}
