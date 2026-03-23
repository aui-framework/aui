uniform {
  mat3 m1
  mat3 m2
  float whichAlgo
}

inter {
  vec2 uv
}

output {
  [0] vec4 albedo
}

entry {
    float m1 = step((uniform.m1 * vec3(inter.uv, 1)).x, 0)
    float m2 = step((uniform.m2 * vec3(inter.uv, 1)).x, 0)
    output.albedo = vec4(1, 1, 1, mix(m1 * m2, m1 + m2, uniform.whichAlgo)) 
    
    #flag glsl120 if (gl_FragColor.a < 0.1) discard;
}