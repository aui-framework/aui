uniform {
  float divider
  float threshold
}

float dashed(float distance) {
  return step(mod(distance, uniform.divider), uniform.threshold);
}