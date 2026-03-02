import dashed

uniform {
  vec4 color
}

inter {
  vec4 vertex
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color * dashed(inter.vertex.z)
}