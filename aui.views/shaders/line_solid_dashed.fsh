import dashed

inter {
  vec4 vertex
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = inter.color * dashed(inter.vertex.z)
}