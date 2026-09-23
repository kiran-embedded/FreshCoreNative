#pragma once

namespace freshcore {
namespace system_tweaks {

// Applies boot-time optimizations (TCP window scaling, read-ahead, etc.)
// Also freezes heavy apps and disables telemetry bloatware.
void ApplyBootTweaks();

// Aggressively forces device idle via dumpsys deviceidle force-idle
void OptimizeDoze();

// Unforces device idle to eliminate lockscreen lag
void WakeFromDoze();

} // namespace system_tweaks
} // namespace freshcore
