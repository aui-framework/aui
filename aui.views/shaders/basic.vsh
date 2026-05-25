input {
    [0] vec2 pos
    [2] vec4 color
}
inter {
    vec4 color
}
uniform {
  mat4 transform
}

entry {
    sl_position = uniform.transform * vec4(input.pos, 0, 1)
    inter.color = input.color
}