#include "vector.h"
#include <math.h>

vec2_t vec2_add(vec2_t a, vec2_t b) { return (vec2_t){a.x + b.x, a.y + b.y}; }

vec2_t vec2_sub(vec2_t a, vec2_t b) { return (vec2_t){a.x - b.x, a.y - b.y}; }

vec3_t vec3_add(vec3_t a, vec3_t b) {
  return (vec3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3_t vec3_sub(vec3_t a, vec3_t b) {
  return (vec3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3_t vec3_scale(vec3_t v, float s) {
  return (vec3_t){v.x * s, v.y * s, v.z * s};
}

float vec3_length(vec3_t v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

vec3_t vec3_normalize(vec3_t v) {
  float l = vec3_length(v);
  return (vec3_t){v.x / l, v.y / l, v.z / l};
}

float vec3_dot(vec3_t a, vec3_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

vec3_t vec3_cross(vec3_t a, vec3_t b) {
  return (vec3_t){
      a.y * b.z - a.z * b.y,
      a.z * b.x - a.x * b.z,
      a.x * b.y - a.y * b.x,
  };
}
float  vec2_dot(vec2_t a, vec2_t b) { return a.x * b.x + a.y * b.y; }
float  vec2_length(vec2_t v) { return sqrtf(v.x * v.x + v.y * v.y); }
vec2_t vec2_scale(vec2_t v, float s) {
  vec2_t result;
  result.x = v.x * s;
  result.y = v.y * s;
  return result;
}
float clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}