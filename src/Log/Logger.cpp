#include <Log/Logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace EXP {

std::shared_ptr<spdlog::logger> Logger::logger;

void Logger::init() {
  spdlog::set_pattern("%^[%T] %n: %v%$");
  logger = spdlog::stdout_color_mt("EXPLORER");
  logger->set_level(spdlog::level::trace);
  logger->debug("Initialized logger.");
}

std::shared_ptr<spdlog::logger> &Logger::get(std::string message) {
  if (!logger)
    Logger::init();
  return logger;
}

void Logger::info(std::string message) {
  if (!logger)
    Logger::init();
  logger->info(message);
}

void Logger::warn(std::string message) {
  if (!logger)
    Logger::init();
  logger->warn(message);
}

void Logger::debug(std::string message) {
  if (!logger)
    Logger::init();
  logger->debug(message);
}

void Logger::error(std::string message) {
  if (!logger)
    Logger::init();
  logger->error(message);
}
} // namespace EXP
