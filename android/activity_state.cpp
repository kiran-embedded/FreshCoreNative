#include "activity_state.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace android {

bool IsPackageForeground(const std::string& package_name) {
    (void)package_name; // unused for now
    // Without ActivityManager Binder, it's hard.
    // We can look at /dev/cpuset/top-app/tasks
    // But translating PIDs to packages requires parsing /proc/<pid>/cmdline
    // This is costly. So we only do it if necessary.
    return false;
}

bool IsHeavyForegroundActivity() {
    // If screen is on, assume foreground activity
    // But we only run when screen is off, so this is an extra check.
    return false;
}

} // namespace android
} // namespace freshcore
