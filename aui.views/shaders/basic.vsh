input {
    [0] vec2 pos
}
uniform {
  mat4 transform
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 0, 1)
}