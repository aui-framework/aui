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
    output.albedo = uniform.color * uniform.albedo[inter.uv]
}