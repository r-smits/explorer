#include <util.h>

NS::String* EXP::nsString(std::string str) {
  return NS::String::string(str.c_str(), NS::StringEncoding::UTF8StringEncoding);
}

NS::URL* EXP::nsUrl(std::string path) {
  return NS::URL::alloc()->initFileURLWithPath(nsString(path));
}

void EXP::printError(NS::Error* error) {
  ERROR(error->debugDescription()->utf8String());
}

void EXP::print(simd::float2 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << "]";
  DEBUG(ss.str());
}

void EXP::print(simd::float3 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << ", " << n.z << "]";
  DEBUG(ss.str());
}
void EXP::print(simd::float4 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << ", " << n.z << ", " << n.w << "]";
  DEBUG(ss.str());
}

void EXP::print(simd::float4x4 m) {

  simd::float4 r1 = simd::float4(0.0f);
  simd::float4 r2 = simd::float4(0.0f);
  simd::float4 r3 = simd::float4(0.0f);
  simd::float4 r4 = simd::float4(0.0f);

  r1.x = m.columns[0].x;
  r2.x = m.columns[0].y;
  r3.x = m.columns[0].z;
  r4.x = m.columns[0].w;

  r1.y = m.columns[1].x;
  r2.y = m.columns[1].y;
  r3.y = m.columns[1].z;
  r4.y = m.columns[1].w;

  r1.z = m.columns[2].x;
  r2.z = m.columns[2].y;
  r3.z = m.columns[2].z;
  r4.z = m.columns[2].w;

  r1.w = m.columns[3].x;
  r2.w = m.columns[3].y;
  r3.w = m.columns[3].z;
  r4.w = m.columns[3].w;

  DEBUG("--simd::float4x4");
  print(r1);
  print(r2);
  print(r3);
  print(r4);
}

void EXP::print(simd::quatf q) {
	std::stringstream ss;
	ss << "(" << q.vector.w << ", " << q.vector.x << ", " << q.vector.y << ", " << q.vector.z << ")";
	DEBUG("--simd::quatf");
	DEBUG(ss.str());
}

std::string EXP::simd2str(const simd::float3 &vec3) {
	return "[x:" + std::to_string(vec3.x) + ", y: " + std::to_string(vec3.y) + ", z: " + std::to_string(vec3.z) + "]";
}

std::string EXP::simd2str(const simd::float4 &vec4) {
	return "[x:" + std::to_string(vec4.x) + ", y: " + std::to_string(vec4.y) + ", z: " + std::to_string(vec4.z) + ", w: " + std::to_string(vec4.w) + "]";
}

