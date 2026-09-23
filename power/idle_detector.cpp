#include "idle_detector.hpp"
#include "../config/config.hpp"
#include "../android/power_state.hpp"
#include "../android/activity_state.hpp"
#include "battery_monitor.hpp"
#include "thermal_monitor.hpp"
#include "../memory/psi_monitor.hpp"

namespace freshcore {
namespace power {

static uint64_t g_screen_off_time = 0;

static uint64_t GetMonotonicTimeSec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec;
}

IdleState EvaluateIdleState() {
    if (android::IsScreenOn()) {
        g_screen_off_time = 0;
        return IdleState::NOT_IDLE;
    }
    
    uint64_t now = GetMonotonicTimeSec();
    if (g_screen_off_time == 0) {
        g_screen_off_time = now;
    }
    
    // Check if the strictly enforced idle delay has passed
    uint64_t required_delay_sec = config::g_config.idle_delay_minutes * 60;
    if (now - g_screen_off_time < required_delay_sec) {
        return IdleState::SCREEN_OFF;
    }
    
    // Are conditions safe?
    if (!IsBatterySafe()) return IdleState::SCREEN_OFF;
    if (!IsThermalSafe()) return IdleState::SCREEN_OFF;
    if (!memory::IsMemoryPressureSafe()) return IdleState::SCREEN_OFF;
    if (android::IsHeavyForegroundActivity()) return IdleState::SCREEN_OFF;

    return IdleState::IDLE_CANDIDATE;
}

} // namespace power
} // namespace freshcore
