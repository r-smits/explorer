#include <util.h>

NS::String* Explorer::nsString(std::string str) {
  return NS::String::string(str.c_str(), NS::StringEncoding::UTF8StringEncoding);
}

NS::URL* Explorer::nsUrl(const std::string& path) {
  return NS::URL::alloc()->initFileURLWithPath(nsString(path));
}

void Explorer::printError(NS::Error* error) {
  ERROR(error->debugDescription()->utf8String());
  ERROR(error->localizedDescription()->utf8String());
  ERROR(error->localizedRecoveryOptions()->debugDescription()->utf8String());
  ERROR(error->localizedFailureReason()->utf8String());
}

void Explorer::print(simd::float2 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << "]";
  DEBUG(ss.str());
}

void Explorer::print(simd::float3 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << ", " << n.z << "]";
  DEBUG(ss.str());
}
void Explorer::print(simd::float4 n) {
  std::stringstream ss;
  ss << "[" << n.x << ", " << n.y << ", " << n.z << ", " << n.w << "]";
  DEBUG(ss.str());
}

void Explorer::print(simd::float4x4 m) {

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

void Explorer::print(simd::quatf q) {
	std::stringstream ss;
	ss << "(" << q.vector.w << ", " << q.vector.x << ", " << q.vector.y << ", " << q.vector.z << ")";
	DEBUG("--simd::quatf");
	DEBUG(ss.str());
}
