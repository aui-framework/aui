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
    output.albedo = inter.color * uniform.albedo[inter.uv]
}
