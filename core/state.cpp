#include "state.hpp"
#include "../logging/logger.hpp"

namespace freshcore {
namespace core {

static EngineState g_current_state = EngineState::BOOT;

const char* StateToString(EngineState state) {
    switch (state) {
        case EngineState::BOOT: return "BOOT";
        case EngineState::ACTIVE: return "ACTIVE";
        case EngineState::SCREEN_OFF: return "SCREEN_OFF";
        case EngineState::IDLE_CANDIDATE: return "IDLE_CANDIDATE";
        case EngineState::DEEP_IDLE: return "DEEP_IDLE";
        case EngineState::MAINTENANCE: return "MAINTENANCE";
        case EngineState::ABORTING: return "ABORTING";
        case EngineState::ERROR_BACKOFF: return "ERROR_BACKOFF";
        default: return "UNKNOWN";
    }
}

void TransitionTo(EngineState new_state) {
    if (g_current_state != new_state) {
        LOGI("State Transition: %s -> %s", StateToString(g_current_state), StateToString(new_state));
        g_current_state = new_state;
    }
}

EngineState GetCurrentState() {
    return g_current_state;
}

} // namespace core
} // namespace freshcore
