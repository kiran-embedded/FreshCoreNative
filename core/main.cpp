#include "core/scheduler.hpp"
#include "config/config.hpp"
#include "logging/logger.hpp"
#include "system/system_tweaks.hpp"
#include "audio/mic_fixer.hpp"
#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    // We write to the internal physical path. 
    // This is safe because KernelSU service.sh runs in a global namespace
    // where /storage/emulated/0 might be an empty tmpfs stub.
    freshcore::logging::Initialize("/data/media/0/Download/FreshCore_Report.txt");
    LOGI("==================================================");
    LOGI("FreshCore Native v1.0 started.");
    LOGI("==================================================");

    if (!freshcore::config::LoadConfig("/data/adb/freshcore/config.conf")) {
        LOGW("Failed to load config, using defaults.");
    }

    if (!freshcore::config::g_config.enabled) {
        LOGI("FreshCore disabled in config. Exiting.");
        freshcore::logging::Shutdown();
        return 0;
    }

    // Apply one-time boot tweaks
    freshcore::system_tweaks::ApplyBootTweaks();

    LOGI("[SYSTEM] Background Maintenance and Cache Cleaner armed.");
    LOGI("[SYSTEM] Sweep will trigger automatically after 15 minutes of Screen-Off Idle.");

    // Start background audio polling
    freshcore::mic_fixer::StartPolling();

    // Start main event loop
    freshcore::core::RunScheduler();

    // Cleanup
    freshcore::mic_fixer::StopPolling();

    freshcore::logging::Shutdown();
    return 0;
}
