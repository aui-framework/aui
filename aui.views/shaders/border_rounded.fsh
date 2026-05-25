import rounded

uniform {
  vec2 innerSize
  vec2 outerToInner
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
    vec2 absolute = abs(inter.uv * 2 - 1)
    output.albedo = inter.color * vec4(1, 1, 1, rounded(absolute, uniform.outerSize) - rounded(absolute * uniform.outerToInner, uniform.innerSize))
#flag glsl120 if (gl_FragColor.a < 0.1) discard;
}
