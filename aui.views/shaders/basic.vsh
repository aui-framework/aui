input {
    [0] vec3 pos
}
uniform {
  mat4 transform
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 1)
}