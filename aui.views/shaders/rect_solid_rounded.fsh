import rounded

uniform {
  vec2 outerSize
}

inter {
  vec2 uv
  vec4 color
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = inter.color * vec4(1, 1, 1, rounded(abs(inter.uv * 2 - 1), uniform.outerSize))
#flag glsl120 if (gl_FragColor.a < 0.1) discard;
}
