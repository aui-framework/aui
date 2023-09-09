import gradient
import rounded

inter {
  vec2 uv
}

uniform {
  vec4 color
  vec2 outerSize
}

output {
  [0] vec4 albedo
}

entry {
    output.albedo = uniform.color * gradient(inter.uv) * vec4(1, 1, 1, rounded(abs(inter.uv * 2 - 1), uniform.outerSize))
}