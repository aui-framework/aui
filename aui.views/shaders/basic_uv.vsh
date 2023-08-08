uniform {
  mat4 transform
}

input {
  [0] vec3 pos
}
inter {
  vec2 uv
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 1)
    inter.uv = input.pos.xy
}