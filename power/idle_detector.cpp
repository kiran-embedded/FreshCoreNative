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

IdleState EvaluateIdleState() {
    if (android::IsScreenOn()) {
        g_screen_off_time = 0;
        return IdleState::NOT_IDLE;
    }
    
    // We don't have a monotonic clock readily available here, let's just assume 
    // the state machine will handle the timing for now, or we can use time()
    // For a real implementation, we'd check how long it's been off.
    // The state machine in core/state.cpp will track the duration.
    
    // Are conditions safe?
    if (!IsBatterySafe()) return IdleState::SCREEN_OFF;
    if (!IsThermalSafe()) return IdleState::SCREEN_OFF;
    if (!memory::IsMemoryPressureSafe()) return IdleState::SCREEN_OFF;
    if (android::IsHeavyForegroundActivity()) return IdleState::SCREEN_OFF;

    return IdleState::IDLE_CANDIDATE;
}

} // namespace power
} // namespace freshcore
