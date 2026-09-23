#include "battery_monitor.hpp"
#include "../logging/logger.hpp"
#include "../config/config.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace power {

static std::string g_battery_capacity_path = "";
static std::string g_battery_status_path = "";

static void DetectBatteryNodes() {
    std::string bases[] = {
        "/sys/class/power_supply/battery",
        "/sys/class/power_supply/bms",
        "/sys/class/power_supply/main"
    };
    
    for (const auto& base : bases) {
        std::ifstream file(base + "/capacity");
        if (file.good()) {
            g_battery_capacity_path = base + "/capacity";
            g_battery_status_path = base + "/status";
            LOGI("Battery nodes found at: %s", base.c_str());
            return;
        }
    }
    LOGW("No known battery sysfs node found.");
}

bool IsBatterySafe() {
    if (g_battery_capacity_path.empty()) {
        DetectBatteryNodes();
    }
    
    if (g_battery_capacity_path.empty()) {
        return false; // Fail-safe: if we don't know the battery, don't maintain
    }
    
    std::ifstream stat_file(g_battery_status_path);
    std::string status;
    bool is_charging = false;
    if (stat_file.is_open() && (stat_file >> status)) {
        if (status == "Charging" || status == "Full") {
            is_charging = true;
        }
    }

    if (config::g_config.charging_preferred && !is_charging) {
        // If charging is preferred but we are not charging, we can still run
        // if battery is above the minimum threshold.
    }
    
    // If charging, it's inherently safe
    if (is_charging) return true;

    std::ifstream cap_file(g_battery_capacity_path);
    int capacity = 0;
    if (cap_file.is_open() && (cap_file >> capacity)) {
        if (capacity > config::g_config.minimum_battery_percent) {
            return true;
        } else {
            LOGW("Aborting: Battery at %d%%, below minimum threshold of %d%%.", capacity, config::g_config.minimum_battery_percent);
        }
    }
    
    return false;
}

} // namespace power
} // namespace freshcore
