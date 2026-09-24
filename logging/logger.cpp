#include "logger.hpp"
#include "../config/config.hpp"
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

namespace freshcore {
namespace logging {

static std::mutex g_log_mutex;
static FILE* g_log_file = nullptr;
static std::string g_log_path;
static size_t g_max_size = 0;

static const char* LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

bool Initialize(const std::string& log_file_path, size_t max_size_bytes) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_file) return true;
    
    g_log_path = log_file_path;
    g_max_size = max_size_bytes;

    // 3-day Auto Delete Logic (3 * 24 * 60 * 60 = 259200 seconds)
    // Absolute Zero-CPU method: Checked only once at initialization.
    struct stat st;
    if (stat(log_file_path.c_str(), &st) == 0) {
        time_t now = time(nullptr);
        if (now - st.st_mtime > 259200) {
            unlink(log_file_path.c_str());
            unlink((log_file_path + ".old").c_str());
        }
    }
    
    g_log_file = fopen(log_file_path.c_str(), "a");
    return g_log_file != nullptr;
}

void Shutdown() {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = nullptr;
    }
}

void Log(LogLevel level, const char* format, ...) {
    if (level == LogLevel::DEBUG && !config::g_config.logging_debug) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (!g_log_file) return;

    // Check size and rotate
    struct stat st;
    if (fstat(fileno(g_log_file), &st) == 0 && (size_t)st.st_size > g_max_size) {
        fclose(g_log_file);
        rename(g_log_path.c_str(), (g_log_path + ".old").c_str());
        g_log_file = fopen(g_log_path.c_str(), "a");
        if (!g_log_file) return;
    }

    time_t now = time(nullptr);
    struct tm tstruct;
    localtime_r(&now, &tstruct);
    
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tstruct);

    fprintf(g_log_file, "[%s] [%s] ", time_buf, LevelToString(level));

    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);

    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

} // namespace logging
} // namespace freshcore
