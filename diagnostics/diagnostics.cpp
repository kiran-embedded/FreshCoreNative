#include "diagnostics.hpp"
#include "../config/config.hpp"
#include "../logging/logger.hpp"
#include <ctime>
#include <fstream>
#include <string>
#include <unistd.h>

namespace freshcore {
namespace diagnostics {

static uint64_t g_batch_start_time_ms = 0;
static int g_total_files_processed_run = 0;
static uint64_t g_total_bytes_freed_run = 0;

static uint64_t GetMonotonicTimeMs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

ResourceUsage GetCurrentUsage() {
    ResourceUsage usage;
    
    // CPU Time
    struct timespec ts;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
        usage.cpu_time_ms = (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
    }

    // RSS from /proc/self/statm (pages)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        uint64_t size, resident;
        if (statm >> size >> resident) {
            long page_size = sysconf(_SC_PAGESIZE);
            usage.rss_kb = (resident * page_size) / 1024;
        }
    }
    
    return usage;
}

bool CheckSelfLimits() {
    ResourceUsage usage = GetCurrentUsage();
    
    // Hardcoded safety limit for daemon memory - 50MB RSS is huge for what we do.
    if (usage.rss_kb > 50 * 1024) {
        LOGE("Self-Limit exceeded: RSS is %llu KB. Aborting.", usage.rss_kb);
        return false;
    }
    
    return true;
}

void StartBatch() {
    g_batch_start_time_ms = GetMonotonicTimeMs();
}

bool EndBatch(int files_processed, uint64_t bytes_freed) {
    uint64_t end_time = GetMonotonicTimeMs();
    uint64_t batch_duration = end_time - g_batch_start_time_ms;
    
    g_total_files_processed_run += files_processed;
    g_total_bytes_freed_run += bytes_freed;
    
    if (batch_duration > (uint64_t)config::g_config.max_batch_time_ms) {
        LOGW("Batch took too long: %llu ms", batch_duration);
        // We don't strictly abort here, just warn. The scheduling logic might delay the next batch.
    }
    
    if (g_total_bytes_freed_run > (uint64_t)config::g_config.max_cleanup_mb_per_run * 1024 * 1024) {
        LOGI("Reached max cleanup MB for this run.");
        return false; // Stop further batches
    }
    
    return CheckSelfLimits();
}

} // namespace diagnostics
} // namespace freshcore
