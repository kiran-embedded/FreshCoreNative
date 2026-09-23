#pragma once
#include <string>

namespace freshcore {
namespace power {

struct BatteryState {
    int percent = -1;
    bool is_charging = false;
    bool present = false;
};

// Reads battery state from /sys/class/power_supply/battery (or similar)
BatteryState GetBatteryState();

// Checks if battery condition is safe for maintenance
bool IsBatterySafe();

} // namespace power
} // namespace freshcore
