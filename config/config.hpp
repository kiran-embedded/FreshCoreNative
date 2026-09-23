#pragma once
#include <string>

namespace freshcore {
namespace config {

struct Configuration {
    bool enabled = true;
    int cache_threshold_mb = 100;
    bool include_user_apps = true;
    bool include_system_apps = true;
    bool include_external_cache = true;
    bool idle_required = true;
    int idle_delay_minutes = 15;
    bool charging_preferred = true;
    int minimum_battery_percent = 25;
    bool thermal_protection = true;
    bool adaptive_scanning = true;
    bool logging_debug = false;
    int max_cleanup_mb_per_run = 2048;
    int max_files_per_batch = 128;
    int max_batch_time_ms = 250;
    int check_interval_active_sec = 1800;
    int check_interval_screen_off_sec = 300;
    int check_interval_deep_idle_sec = 60;
};

// Global config instance
extern Configuration g_config;

// Load config from /data/adb/freshcore/config.conf
bool LoadConfig(const std::string& path);

} // namespace config
} // namespace freshcore
