#pragma once

namespace freshcore {
namespace core {

enum class EngineState {
    BOOT,
    ACTIVE,
    SCREEN_OFF,
    IDLE_CANDIDATE,
    DEEP_IDLE,
    MAINTENANCE,
    ABORTING,
    ERROR_BACKOFF
};

const char* StateToString(EngineState state);

// Update internal state machine
void TransitionTo(EngineState new_state);

// Get current state
EngineState GetCurrentState();

} // namespace core
} // namespace freshcore
