uniform {
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color
}