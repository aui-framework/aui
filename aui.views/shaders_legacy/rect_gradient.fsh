import gradient

inter {
  vec2 uv
}

uniform {
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color * gradient(inter.uv)
}