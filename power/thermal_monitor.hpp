#pragma once

namespace freshcore {
namespace power {

// Reads maximum temperature across available thermal zones
int GetMaxTemperatureC();

// Checks if device is thermally safe for maintenance
bool IsThermalSafe();

} // namespace power
} // namespace freshcore
