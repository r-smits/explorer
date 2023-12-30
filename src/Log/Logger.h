#pragma once
#include <pch.h>
#include <spdlog/spdlog.h>

namespace Explorer {

class Logger {
public:
  static void init();
  static std::shared_ptr<spdlog::logger> &get(std::string message);
  static void debug(std::string message);
  static void info(std::string message);
  static void warn(std::string message);
  static void error(std::string message);

private:
  static std::shared_ptr<spdlog::logger> logger;
};
} // namespace Explorer

#define DEBUG(...) ::Explorer::Logger::debug(__VA_ARGS__)
#define INFO(...) ::Explorer::Logger::info(__VA_ARGS__)
#define WARN(...) ::Explorer::Logger::warn(__VA_ARGS__)
#define ERROR(...) ::Explorer::Logger::error(__VA_ARGS__)
