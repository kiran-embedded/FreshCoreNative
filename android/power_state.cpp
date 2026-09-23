#include "power_state.hpp"
#include "../logging/logger.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace android {

static bool g_screen_supported = true;
static std::string g_backlight_path = "";

static void DetectScreenNode() {
    // Layered detection:
    std::string paths[] = {
        "/sys/class/backlight/panel0-backlight/brightness", // Common QC
        "/sys/class/leds/lcd-backlight/brightness",         // MTK
        "/sys/class/backlight/intel_backlight/brightness",
        "/sys/class/backlight/acpi_video0/brightness"
    };
    
    for (const auto& path : paths) {
        std::ifstream file(path);
        if (file.good()) {
            g_backlight_path = path;
            LOGI("Screen state fallback node found: %s", path.c_str());
            return;
        }
    }
    
    LOGW("No known screen brightness sysfs node found. Screen detection may be unsupported.");
    g_screen_supported = false; // We can fallback to Binder, but sysfs is missing
}

bool IsScreenOn() {
    if (g_backlight_path.empty() && g_screen_supported) {
        DetectScreenNode();
    }
    
    if (!g_backlight_path.empty()) {
        std::ifstream file(g_backlight_path);
        if (file.is_open()) {
            int brightness = 0;
            if (file >> brightness) {
                return brightness > 0;
            }
        }
    }
    
    // 100% Universal Fallback: Use dumpsys power
    std::string result = "";
    FILE* pipe = popen("dumpsys power 2>/dev/null | grep -m 1 'mWakefulness='", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result = buffer;
        }
        pclose(pipe);
    }
    
    if (result.find("Asleep") != std::string::npos || result.find("Dozing") != std::string::npos) {
        return false; // Screen is off
    }
    
    // Fail-safe: Assume on
    return true;
}

} // namespace android
} // namespace freshcore
