import rounded

uniform {
  vec4 color
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color * vec4(1, 1, 1, rounded(inter.uv))
}