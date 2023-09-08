import rounded

uniform {
  vec4 color
  vec2 innerSize
  vec2 outerToInner
  vec2 outerSize
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    vec2 absolute = abs(inter.uv * 2 - 1)
    output.albedo = uniform.color * vec4(1, 1, 1, rounded(absolute, uniform.outerSize) - rounded(absolute * uniform.outerToInner, uniform.innerSize))
}