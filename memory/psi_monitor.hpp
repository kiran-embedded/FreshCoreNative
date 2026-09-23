#pragma once

namespace freshcore {
namespace memory {

struct PSIStats {
    float some_avg10 = 0.0f;
    float full_avg10 = 0.0f;
    bool is_supported = false;
};

// Gets Memory PSI (Pressure Stall Information)
PSIStats GetMemoryPSI();

// Checks if memory pressure is low enough for safe maintenance
bool IsMemoryPressureSafe();

} // namespace memory
} // namespace freshcore
