#include "config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace freshcore {
namespace config {

Configuration g_config;

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool LoadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t delim_pos = line.find('=');
        if (delim_pos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, delim_pos));
        std::string value = trim(line.substr(delim_pos + 1));

        if (key == "enabled") g_config.enabled = (value == "true" || value == "1");
        else if (key == "cache_threshold_mb") g_config.cache_threshold_mb = std::atoi(value.c_str());
        else if (key == "include_user_apps") g_config.include_user_apps = (value == "true" || value == "1");
        else if (key == "include_system_apps") g_config.include_system_apps = (value == "true" || value == "1");
        else if (key == "include_external_cache") g_config.include_external_cache = (value == "true" || value == "1");
        else if (key == "idle_required") g_config.idle_required = (value == "true" || value == "1");
        else if (key == "idle_delay_minutes") g_config.idle_delay_minutes = std::atoi(value.c_str());
        else if (key == "charging_preferred") g_config.charging_preferred = (value == "true" || value == "1");
        else if (key == "minimum_battery_percent") g_config.minimum_battery_percent = std::atoi(value.c_str());
        else if (key == "thermal_protection") g_config.thermal_protection = (value == "true" || value == "1");
        else if (key == "adaptive_scanning") g_config.adaptive_scanning = (value == "true" || value == "1");
        else if (key == "logging") g_config.logging_debug = (value == "true" || value == "1");
        else if (key == "max_cleanup_mb_per_run") g_config.max_cleanup_mb_per_run = std::atoi(value.c_str());
        else if (key == "max_files_per_batch") g_config.max_files_per_batch = std::atoi(value.c_str());
        else if (key == "max_batch_time_ms") g_config.max_batch_time_ms = std::atoi(value.c_str());
    }
    return true;
}

} // namespace config
} // namespace freshcore
