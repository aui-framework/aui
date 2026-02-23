input {
    [0] vec3 pos
}
uniform {
  mat4 transform
}

entry {
    vec4 position = uniform.transform * vec4(input.pos, 1)
    position.z = input.pos.z
    sl_position = position
}
