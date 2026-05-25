import gradient

inter {
  vec2 uv
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = inter.color * gradient(inter.uv)
}