uniform {
  2D albedo
}

inter {
  vec2 uv
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    vec4 s = uniform.albedo[inter.uv]
    output.albedo = inter.color * vec4(1, 1, 1, s.x)
}
