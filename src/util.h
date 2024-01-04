#pragma once
#include <pch.h>

namespace Explorer {
NS::String* nsString(std::string str);
NS::URL* nsUrl(std::string path);
void printError(NS::Error* error);
}; // namespace Explorer
