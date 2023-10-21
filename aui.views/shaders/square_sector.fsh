uniform {
  float begin
  float end 
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = vec4(1, 1, 1, step(inter.uv.y, 0.5)) 
    #flag glsl120 if (gl_FragColor.a < 0.1) discard;
}