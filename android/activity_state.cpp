#include "activity_state.hpp"
#include "power_state.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace android {

bool IsPackageForeground(const std::string& package_name) {
    FILE* pipe = popen("dumpsys window | grep -m 1 'mCurrentFocus'", "r");
    if (!pipe) return false;
    
    char buffer[128];
    bool found = false;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string current_focus(buffer);
        if (current_focus.find(package_name) != std::string::npos) {
            found = true;
        }
    }
    pclose(pipe);
    return found;
}

bool IsHeavyForegroundActivity() {
    // If screen is off, there is no foreground activity
    if (!IsScreenOn()) return false;

    FILE* pipe = popen("dumpsys window | grep -m 1 'mCurrentFocus'", "r");
    if (!pipe) return false;
    
    char buffer[128];
    bool is_heavy = false;
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string current_focus(buffer);
        // List of heavy apps that should block maintenance
        const char* heavy_apps[] = {
            "whatsapp", "facebook", "instagram", "tiktok", "snapchat", 
            "telegram", "youtube", "camera", "pubg", "callofduty", 
            "genshin", "discord", "netflix", "maps", "chrome", "viber"
        };
        for (const char* app : heavy_apps) {
            if (current_focus.find(app) != std::string::npos) {
                is_heavy = true;
                break;
            }
        }
    }
    pclose(pipe);
    return is_heavy;
}

} // namespace android
} // namespace freshcore
