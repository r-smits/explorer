#include <Math/Transformation.h>

float toRadians(float degrees) { return degrees * (M_PI / 180); }
simd::float4x4 Transformation::identity() { return simd::float4x4(1.0f); }

simd::float4x4 Transformation::translation(simd::float3 pos) {
  simd::float4 column1 = {1.0f, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, 1.0f, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd::float4 column4 = {pos[0], pos[1], pos[2], 1.0f};
  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::zRotation(float theta) {
  theta = toRadians(theta); // Degrees -> radians: degrees * (pi/180)
  float cos = cosf(theta);
  float sin = sinf(theta);

  simd::float4 column1 = {cos, sin, 0.0f, 0.0f};
  simd::float4 column2 = {-sin, cos, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::xRotation(float theta) {
  theta = theta * (M_PI / 180);
  float cos = cosf(theta);
  float sin = sinf(theta);

  simd::float4 column1 = {1.0f, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, cos, -sin, 0.0f};
  simd::float4 column3 = {0.0f, sin, cos, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::yRotation(float theta) {
  theta = theta * (M_PI / 180);
  float cos = cosf(theta);
  float sin = sinf(theta);

  simd::float4 column1 = {cos, 0.0f, sin, 0.0f};
  simd::float4 column2 = {0.0f, 1.0f, 0.0f, 0.0f};
  simd::float4 column3 = {-sin, 0.0f, cos, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::scale(float factor) {
  simd_float4 column1 = {factor, 0.0f, 0.0f, 0.0f};
  simd_float4 column2 = {0.0f, factor, 0.0f, 0.0f};
  simd_float4 column3 = {0.0f, 0.0f, factor, 0.0f};
  simd_float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 Transformation::perspective(float fov, float aspectRatio, float nearZ, float farZ) {
  float yPerspective = 1 / (tanf(toRadians(fov) * 0.5f));
  float xPerspective = yPerspective / aspectRatio;
  float zPerspective = farZ / (nearZ - farZ);
  float wPerspective = zPerspective * nearZ;

  simd::float4 column1 = {xPerspective, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, yPerspective, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, zPerspective, -1.0f};
  simd::float4 column4 = {0.0f, 0.0f, wPerspective, 0.0f};
  return simd::float4x4(column1, column2, column3, column4);
}

simd::float4x4 Transformation::orthographic(
    float left, float right, float bottom, float top, float nearZ, float farZ
) {
  simd::float4 col1 = {2.0f / (right - left), 0.0f, 0.0f, 0.0f};
  simd::float4 col2 = {0.0f, 2.0f / (top - bottom), 0.0f, 0.0f};
  simd::float4 col3 = {0.0f, 0.0f, 1.0f / (farZ - nearZ), 0.0f};
  simd::float4 col4 = {
      -(right + left) / (right - left),
      -(top + bottom) / (top - bottom),
      -nearZ / (farZ - nearZ),
      1.0f
  };
  return simd::float4x4(col1, col2, col3, col4);
};
