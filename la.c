#include "la.h"

Vec2f vec2f(float x, float y) { return (Vec2f){.x = x, .y = y}; }

Vec2f vec2fs(float x) { return (Vec2f){.x = x, .y = x}; }

Vec2f vec2f_add(Vec2f a, Vec2f b) {
  return (Vec2f){.x = a.x + b.x, .y = a.y + b.y};
}

Vec2f vec2f_sub(Vec2f a, Vec2f b) {
  return (Vec2f){.x = a.x - b.x, .y = a.y - b.y};
}

Vec2f vec2f_mul(Vec2f a, Vec2f b) {
  return (Vec2f){.x = a.x * b.x, .y = a.y * b.y};
}

Vec2f vec2f_mul3(Vec2f a, Vec2f b, Vec2f c) {
  return vec2f_mul(vec2f_mul(a, b), c);
}

Vec2f vec2f_div(Vec2f a, Vec2f b) {
  return (Vec2f){.x = a.x / b.x, .y = a.y / b.y};
}

Vec2i vec2i(int x, int y) { return (Vec2i){.x = x, .y = y}; }

Vec2i vec2is(int x) { return (Vec2i){.x = x, .y = x}; }

Vec2i vec2i_add(Vec2i a, Vec2i b) {
  return (Vec2i){.x = a.x + b.x, .y = a.y + b.y};
}

Vec2i vec2i_sub(Vec2i a, Vec2i b) {
  return (Vec2i){.x = a.x - b.x, .y = a.y - b.y};
}

Vec2i vec2i_mul(Vec2i a, Vec2i b) {
  return (Vec2i){.x = a.x * b.x, .y = a.y * b.y};
}

Vec2i vec2i_mul3(Vec2i a, Vec2i b, Vec2i c) {
  return vec2i_mul(vec2i_mul(a, b), c);
}

Vec2i vec2i_div(Vec2i a, Vec2i b) {
  return (Vec2i){.x = a.x / b.x, .y = a.y / b.y};
}

Vec4f vec4f(float x, float y, float z, float w) {
  return (Vec4f){.x = x, .y = y, .z = z, .w = w};
}

Vec4f vec4fs(float x) { return (Vec4f){.x = x, .y = x, .z = x, .w = x}; }

Vec4f vec4f_add(Vec4f a, Vec4f b) {
  return (Vec4f){
      .x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
}

Vec4f vec4f_sub(Vec4f a, Vec4f b) {
  return (Vec4f){
      .x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w};
}

Vec4f vec4f_mul(Vec4f a, Vec4f b) {
  return (Vec4f){
      .x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z, .w = a.w * b.w};
}

Vec4f vec4f_div(Vec4f a, Vec4f b) {
  return (Vec4f){
      .x = a.x / b.x, .y = a.y / b.y, .z = a.z / b.z, .w = a.w / b.w};
}
