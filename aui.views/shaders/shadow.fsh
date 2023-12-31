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
  vec4 vertex
}

vec4 erf(vec4 x) {
  vec4 s = sign(x)
  vec4 a = abs(x)
  x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a
  x = x * x
  return s - s / (x * x)
}


entry {
  vec4 result = uniform.color

  vec2 v = inter.vertex.xy
  vec4 query = vec4(v - vec2(uniform.lower), v - vec2(uniform.upper))
  vec4 integral = 0.5 + 0.5 * erf(query * (sqrt(0.5) / uniform.sigma))
  result.a = result.a * clamp((integral.z - integral.x) * (integral.w - integral.y), 0.0, 1.0)
  output.albedo = result
}