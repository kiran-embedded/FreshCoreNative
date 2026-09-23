#include "system_tweaks.hpp"
#include "../logging/logger.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace freshcore {
namespace system_tweaks {

static void WriteToFile(const std::string& path, const std::string& val) {
    std::ofstream f(path);
    if (f) {
        f << val;
    }
}

void ApplyBootTweaks() {
    LOGI("Applying Boot-Time System Tweaks...");

    // 1. TCP Window Scaling & Memory Limits
    WriteToFile("/proc/sys/net/ipv4/tcp_window_scaling", "1");
    WriteToFile("/proc/sys/net/ipv4/tcp_rmem", "4096 87380 16777216");
    WriteToFile("/proc/sys/net/ipv4/tcp_wmem", "4096 16384 16777216");

    // 2. I/O Storage Speed Boost (Read-Ahead Buffer)
    std::error_code ec;
    if (fs::exists("/sys/block", ec)) {
        for (const auto& entry : fs::directory_iterator("/sys/block", ec)) {
            if (entry.is_directory(ec)) {
                std::string path = entry.path().string() + "/queue/read_ahead_kb";
                if (fs::exists(path, ec)) {
                    WriteToFile(path, "2048");
                }
            }
        }
    }

    // 3. Background Native App Compiler (Executed via shell)
    // We run these asynchronously in the background so we don't block our event loop
    std::system(
        "( "
        "cmd package bg-dexopt-job >/dev/null 2>&1; "
        ") &"
    );

    LOGI("Boot-Time System Tweaks applied.");
}

void OptimizeDoze() {
    LOGI("Pushing system into Extreme Doze Mode (force-idle)...");
    // We execute this asynchronously so it doesn't block
    std::system("dumpsys deviceidle force-idle >/dev/null 2>&1 &");
    LOGI("Deep Doze engaged.");
}

void WakeFromDoze() {
    LOGI("Waking system from Doze to prevent lockscreen lag...");
    // Instantly drops deep doze constraints
    std::system("dumpsys deviceidle unforce >/dev/null 2>&1 &");
}

} // namespace system_tweaks
} // namespace freshcore
