#pragma once
#include <cstdint>
#include <sys/types.h>

namespace freshcore {
namespace diagnostics {

struct ResourceUsage {
    uint64_t cpu_time_ms = 0;
    uint64_t rss_kb = 0;
};

// Check if the current process is exceeding safe limits
bool CheckSelfLimits();

// Get current process CPU time and RSS
ResourceUsage GetCurrentUsage();

// Call this before starting a batch
void StartBatch();

// Call this after a batch. Increments internal counters and checks limits.
bool EndBatch(int files_processed, uint64_t bytes_freed);

} // namespace diagnostics
} // namespace freshcore
