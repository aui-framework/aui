uniform {
  vec4 color
  vec2 lower
  vec2 upper
  float sigma
}

output {
  [0] vec4 albedo
}

inter {
  vec2 uv
}

entry {
}