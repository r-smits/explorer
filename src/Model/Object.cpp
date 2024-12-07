#include <Model/Object.h>

EXP::Object::Object()
    : orientation(simd::float4x4(1)), position(simd::float3(0)), rotation(simd::float4x4(1)),
      factor(1) {}

// Computation of matrices are down right -> left.
// Meaning you first need to translate, then rotate, then scale
EXP::Object* EXP::Object::f4x4() {
  orientation = EXP::MATH::translation(position) * rotation * EXP::MATH::scale(factor);
  return this;
}
EXP::Object* EXP::Object::translate(const simd::float3& position) {
  this->position += position;
  return this;
}

EXP::Object* EXP::Object::scale(const float& factor) {
  this->factor = factor;
  return this;
}
EXP::Object* EXP::Object::rotate(const simd::float4x4& rotation) {
  this->rotation = rotation;
  return this;
}


