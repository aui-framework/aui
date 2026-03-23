uniform {
  vec4 color
  2D albedo
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    vec4 s = uniform.albedo[inter.uv]
    output.albedo = uniform.color * vec4(1, 1, 1, s.x)
}
