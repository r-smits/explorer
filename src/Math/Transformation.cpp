#include "Metal/MTLAccelerationStructureTypes.hpp"
#include <Math/Transformation.h>

float toRadians(const float& degrees) { return degrees * (M_PI / 180); }
simd::float4x4 EXP::MATH::identity() { return simd::float4x4(1.0f); }

simd::float4x4 EXP::MATH::translation(const simd::float3& pos) {
  simd::float4 column1 = {1.0f, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, 1.0f, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd::float4 column4 = {pos[0], pos[1], pos[2], 1.0f};
  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 EXP::MATH::zRotation(const float& theta) {
  float rad = toRadians(theta); // Degrees -> radians: degrees * (pi/180)
  float cos = cosf(rad);
  float sin = sinf(rad);

  simd::float4 column1 = {cos, sin, 0.0f, 0.0f};
  simd::float4 column2 = {-sin, cos, 0.0f, 0.0f};
  simd::float4 column3 = {0.0f, 0.0f, 1.0f, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 EXP::MATH::xRotation(const float& theta) {
	float rad = toRadians(theta); // Degrees -> radians: degrees * (pi/180)
  float cos = cosf(rad);
  float sin = sinf(rad);

  simd::float4 column1 = {1.0f, 0.0f, 0.0f, 0.0f};
  simd::float4 column2 = {0.0f, cos, -sin, 0.0f};
  simd::float4 column3 = {0.0f, sin, cos, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 EXP::MATH::yRotation(const float& theta) {
  float totalTheta = theta * (M_PI / 180);
  float cos = cosf(totalTheta);
  float sin = sinf(totalTheta);

  simd::float4 column1 = {cos, 0.0f, sin, 0.0f};
  simd::float4 column2 = {0.0f, 1.0f, 0.0f, 0.0f};
  simd::float4 column3 = {-sin, 0.0f, cos, 0.0f};
  simd::float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 EXP::MATH::scale(const float& factor) {
  simd_float4 column1 = {factor, 0.0f, 0.0f, 0.0f};
  simd_float4 column2 = {0.0f, factor, 0.0f, 0.0f};
  simd_float4 column3 = {0.0f, 0.0f, factor, 0.0f};
  simd_float4 column4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(column1, column2, column3, column4);
}

simd::float4x4 EXP::MATH::perspective(const float& fov, const float& aspectRatio, const float& nearZ, const float& farZ) {
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

simd::float4x4 EXP::MATH::orthographic(
    const float& left, const float& right, const float& bottom, const float& top, const float& nearZ, const float& farZ
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

// This is for right handed vertices!
simd::float4x4 EXP::MATH::lookat(const simd::float3& eye, const simd::float3& center, const simd::float3& up) {

  // const simd::float3 sF = simd::normalize(sCenter - sEye);
  // const simd::float3 sS = simd::normalize(simd::cross(sF, vUp));
  // const simd::float3 sU = simd::cross(sS, sF);

  const simd::float3 f = simd::normalize(center - eye);
  const simd::float3 s = simd::normalize(simd::cross(f, up));
  const simd::float3 u = simd::cross(s, f);

  // The GLM variant
  // const simd::float4 col1 = {s.x, u.x, -f.x, -simd::dot(s, eye)};
  // const simd::float4 col2 = {s.y, u.y, -f.y, -simd::dot(u, eye)};
  // const simd::float4 col3 = {s.z, u.z, -f.z, simd::dot(f, eye)};
  // const simd::float4 col4 = {0.0f, 0.0f, 0.0f, 1.0f};

  const simd::float4 col1 = {s.x, s.y, s.z, -simd::dot(s, eye)};
  const simd::float4 col2 = {u.x, u.y, u.z, -simd::dot(u, eye)};
  const simd::float4 col3 = {-f.x, -f.y, -f.z, simd::dot(f, eye)};
  const simd::float4 col4 = {0.0f, 0.0f, 0.0f, 1.0f};
  // const simd::float4 col4 = {-simd::dot(s, eye), -simd::dot(u, eye),
  // simd::dot(f, eye), 1.0f};
  return simd_matrix(col1, col2, col3, col4);
}

// Alternative
simd::float4x4 EXP::MATH::lookat2(const simd::float3& right, const simd::float3& center, const simd::float3& up) {
  return simd::float4x4(1.0f);
}

simd::float4x4 EXP::MATH::rotate(const float& angle, const simd::float3& vec) {

  simd::float3 axis = simd::normalize(vec);

  float a = angle;

  float c = cos(a);
  float s = sin(a);

  float x = axis.x;
  float y = axis.y;
  float z = axis.z;

  float mc = (1 - c);

  simd::float4 col1 = {x * x * mc + c, x * y * mc + z * s, x * z * mc - y * s, 0.0f};
  simd::float4 col2 = {y * x * mc - z * s, y * y * mc + c, y * z * mc + x * s, 0.0f};
  simd::float4 col3 = {z * x * mc + y * s, z * y * mc - x * s, z * z * mc + c, 0.0f};
  simd::float4 col4 = {0.0f, 0.0f, 0.0f, 1.0f};

  return simd_matrix(col1, col2, col3, col4);
}

simd::quatf EXP::MATH::cross(const simd::quatf& a, const simd::quatf& b) {

  simd::float4 q1 = a.vector;
  simd::float4 q2 = b.vector;

  simd::quatf q = simd::quatf(0.0f);
  q.vector.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
  q.vector.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
  q.vector.y = q1.w * q2.y + q1.y * q2.w + q1.z * q2.x - q1.x * q2.z;
  q.vector.z = q1.w * q2.z + q1.z * q2.w + q1.x * q2.y - q1.y * q2.x;

  return q;
}

MTL::PackedFloat4x3 EXP::MATH::pack(const simd::float4x4& m4x4) {
	MTL::PackedFloat3 col1 = {m4x4.columns[0].x, m4x4.columns[0].y, m4x4.columns[0].z};
	MTL::PackedFloat3 col2 = {m4x4.columns[1].x, m4x4.columns[1].y, m4x4.columns[1].z};
	MTL::PackedFloat3 col3 = {m4x4.columns[2].x, m4x4.columns[2].y, m4x4.columns[2].z};
	MTL::PackedFloat3 col4 = {m4x4.columns[3].x, m4x4.columns[3].y, m4x4.columns[3].z};
	MTL::PackedFloat4x3 result = {col1, col2, col3, col4};
	return result;
}

/**
MTLPackedFloat3 packedFloat3;
    packedFloat3.x = x;
    packedFloat3.y = y;
    packedFloat3.z = z;
    return packedFloat3;
}
**/

