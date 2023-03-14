input {
    [0] vec3 pos
}

entry {
    sl_position = vec4(input.pos, 1)
}