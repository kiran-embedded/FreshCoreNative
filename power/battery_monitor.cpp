#include "battery_monitor.hpp"
#include "../logging/logger.hpp"
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
    
    std::ifstream cap_file(g_battery_capacity_path);
    int capacity = 0;
    if (cap_file.is_open() && (cap_file >> capacity)) {
        if (capacity > 20) { // arbitrary threshold
            return true;
        }
    }
    
    std::ifstream stat_file(g_battery_status_path);
    std::string status;
    if (stat_file.is_open() && (stat_file >> status)) {
        if (status == "Charging" || status == "Full") {
            return true;
        }
    }
    
    return false;
}

} // namespace power
} // namespace freshcore
