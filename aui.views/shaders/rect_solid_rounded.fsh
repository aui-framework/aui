import rounded

uniform {
  vec4 color
  vec2 outerSize
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color * vec4(1, 1, 1, rounded(abs(inter.uv * 2 - 1), uniform.outerSize))
}