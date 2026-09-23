#pragma once
#include <string>

namespace freshcore {
namespace logging {

enum class LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR
};

// Initialize file logger.
bool Initialize(const std::string& log_file_path, size_t max_size_bytes = 2 * 1024 * 1024); // 2MB max
void Shutdown();
void Log(LogLevel level, const char* format, ...);

} // namespace logging
} // namespace freshcore

#define LOGE(...) freshcore::logging::Log(freshcore::logging::LogLevel::ERROR, __VA_ARGS__)
#define LOGW(...) freshcore::logging::Log(freshcore::logging::LogLevel::WARNING, __VA_ARGS__)
#define LOGI(...) freshcore::logging::Log(freshcore::logging::LogLevel::INFO, __VA_ARGS__)
#define LOGD(...) freshcore::logging::Log(freshcore::logging::LogLevel::DEBUG, __VA_ARGS__)
