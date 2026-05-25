uniform {
  2D albedo
}

inter {
  vec2 uv
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    vec4 s = uniform.albedo[inter.uv]
    vec4 c = inter.color
    output.albedo = vec4(c.xyz * s.xyz * c.a, (s.x + s.y + s.z) / 3 * c.a)
}
