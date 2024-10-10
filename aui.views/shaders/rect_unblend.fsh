uniform {
  vec4 color
  2D albedo
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    vec4 unblend = uniform.albedo[inter.uv]
    unblend.xyz = unblend.xyz / unblend.a
    output.albedo = uniform.color * unblend
}
