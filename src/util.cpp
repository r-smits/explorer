#include <util.h>

NS::String* Explorer::nsString(std::string str) {
  return NS::String::string(str.c_str(), NS::StringEncoding::UTF8StringEncoding);
}

NS::URL* Explorer::nsUrl(std::string path) {
return NS::URL::alloc()->initFileURLWithPath(nsString(path));
}

void Explorer::printError(NS::Error* error) {
  ERROR(error->debugDescription()->utf8String());
  ERROR(error->localizedDescription()->utf8String());
  ERROR(error->localizedRecoveryOptions()->debugDescription()->utf8String());
  ERROR(error->localizedFailureReason()->utf8String());
}
