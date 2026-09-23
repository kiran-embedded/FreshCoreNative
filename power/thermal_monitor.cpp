#include "thermal_monitor.hpp"
#include "../logging/logger.hpp"
#include "../config/config.hpp"
#include <fstream>
#include <dirent.h>
#include <string>
#include <vector>

namespace freshcore {
namespace power {

static std::vector<std::string> g_thermal_zones;

static void DetectThermalZones() {
    DIR* dir = opendir("/sys/class/thermal");
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name.find("thermal_zone") == 0) {
            std::string path = "/sys/class/thermal/" + name + "/temp";
            std::ifstream file(path);
            if (file.good()) {
                g_thermal_zones.push_back(path);
            }
        }
    }
    closedir(dir);
    LOGI("Detected %zu thermal zones.", g_thermal_zones.size());
}

bool IsThermalSafe() {
    if (!config::g_config.thermal_protection) {
        return true; // User disabled thermal protection
    }

    if (g_thermal_zones.empty()) {
        DetectThermalZones();
    }
    
    if (g_thermal_zones.empty()) {
        // Fail-safe: no thermal zones? We don't know the temp.
        // It's safer to defer maintenance.
        return false;
    }
    
    for (const auto& path : g_thermal_zones) {
        std::ifstream file(path);
        int temp = 0;
        if (file.is_open() && (file >> temp)) {
            // Some devices report millidegrees (45000), some report degrees (45)
            if (temp > 1000) {
                temp /= 1000;
            }
            if (temp > 45) { // 45C threshold
                return false;
            }
        }
    }
    
    return true;
}

} // namespace power
} // namespace freshcore
