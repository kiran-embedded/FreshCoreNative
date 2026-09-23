#pragma once

namespace freshcore {
namespace power {

enum class IdleState {
    NOT_IDLE = 0,
    SCREEN_OFF,
    IDLE_CANDIDATE,
    DEEP_IDLE
};

// Evaluate the current overall idle state of the device
IdleState EvaluateIdleState();

} // namespace power
} // namespace freshcore
