#include "core/scheduler.hpp"
#include "config/config.hpp"
#include "logging/logger.hpp"
#include <iostream>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    freshcore::logging::Initialize("/data/local/tmp/freshcore.log");
    LOGI("FreshCore Native starting up...");

    if (!freshcore::config::LoadConfig("/data/adb/freshcore/config.conf")) {
        LOGW("Failed to load config, using defaults.");
    }

    if (!freshcore::config::g_config.enabled) {
        LOGI("FreshCore disabled in config. Exiting.");
        freshcore::logging::Shutdown();
        return 0;
    }

    freshcore::core::RunScheduler();

    freshcore::logging::Shutdown();
    return 0;
}
