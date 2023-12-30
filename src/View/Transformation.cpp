#include <View/Transformation.h>

simd::float4x4 Transformation::identity() { return simd::float4x4(1.0f); }

simd::float4x4 Transformation::translation(simd::float3 pos) {
  simd::float4 column1 = {1.0f, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, 1.0f, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd::float4 column4 = {pos[0], pos[1], pos[2], 1.0f};
  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::zRotation(float theta) {
  theta = theta * 180 / M_PI;
  float c = cosf(theta);
  float s = sinf(theta);

  simd::float4 column1 = {c, s, 0.0f, 0.0f};
  simd_float4 column2 = {-s, c, 0.0f, 0.0f};
  simd_float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd_float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::scale(float factor) {
  simd_float4 column1 = {factor, 0.0f, 0.0f, 0.0f};
  simd_float4 column2 = {0.0f, factor, 0.0f, 0.0f};
  simd_float4 column3 = {0.0f, 0.0f, factor, 0.0f};
  simd_float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}
