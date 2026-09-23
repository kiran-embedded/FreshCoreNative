#include "cache_analyzer.hpp"
#include "cache_policy.hpp"
#include "state_storage.hpp"
#include "../logging/logger.hpp"
#include <algorithm>
#include <ctime>

namespace freshcore {
namespace cache {

static uint64_t GetCurrentTimeMs() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

std::vector<MaintenanceTarget> AnalyzeCaches(std::vector<CacheDirInfo>& scanned_dirs) {
    std::vector<MaintenanceTarget> targets;
    uint64_t now = GetCurrentTimeMs();
    
    // Cooldown logic parameters
    const uint64_t MIN_CLEANUP_INTERVAL_MS = 6 * 60 * 60 * 1000ULL; // 6 hours
    
    for (auto& dir : scanned_dirs) {
        PackageState state = GetPackageState(dir.pkg_name);
        
        // Check cooldown
        if (now > state.last_cleanup_ms && (now - state.last_cleanup_ms) < MIN_CLEANUP_INTERVAL_MS) {
            // Wait before checking this package again unless growth rate is massive
            if (state.growth_rate_bytes_per_day < 500ULL * 1024ULL * 1024ULL) {
                continue; // Skip this package for now
            }
        }
        
        // Stage 2 scanning: measure size
        CalculateCacheSize(dir);
        
        // Calculate time diff and growth
        uint64_t time_diff_ms = now > state.last_scan_ms ? (now - state.last_scan_ms) : 0;
        uint64_t time_diff_days = time_diff_ms / (24 * 60 * 60 * 1000ULL);
        
        // If we have previous state and some days have passed, update growth rate
        if (time_diff_days > 0 && dir.size_bytes > state.last_size_bytes) {
            state.growth_rate_bytes_per_day = (dir.size_bytes - state.last_size_bytes) / time_diff_days;
        }
        
        // Update state with new scan time and size
        state.last_scan_ms = now;
        state.last_size_bytes = dir.size_bytes;
        UpdatePackageState(dir.pkg_name, state);
        
        if (ShouldCleanCache(dir.size_bytes)) {
            MaintenanceTarget target;
            target.cache_dir = dir;
            
            // Adjust priority based on size and growth
            target.priority = 1;
            if (dir.size_bytes > 1024ULL * 1024ULL * 1024ULL) {
                target.priority += 2;
            } else if (dir.size_bytes > 500ULL * 1024ULL * 1024ULL) {
                target.priority += 1;
            }
            if (state.growth_rate_bytes_per_day > 250ULL * 1024ULL * 1024ULL) {
                target.priority += 1; // Fast grower
            }
            
            targets.push_back(target);
        }
    }
    
    // Save state after analyzing
    SaveStateAtomic();
    
    // Sort by priority descending
    std::sort(targets.begin(), targets.end(), [](const MaintenanceTarget& a, const MaintenanceTarget& b) {
        return a.priority > b.priority;
    });
    
    return targets;
}

} // namespace cache
} // namespace freshcore
