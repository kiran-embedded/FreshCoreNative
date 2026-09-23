#include "cache_cleaner.hpp"
#include "state_storage.hpp"
#include "../config/config.hpp"
#include "../diagnostics/diagnostics.hpp"
#include "../storage/filesystem.hpp"
#include "../logging/logger.hpp"
#include "../power/idle_detector.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>

namespace freshcore {
namespace cache {

static uint64_t GetCurrentTimeMs() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}

static bool RemoveRecursiveBatched(const std::string& path) {
    std::vector<std::string> children;
    if (!storage::GetDirectoryChildren(path, children)) {
        return false; // Not a dir or can't read
    }
    
    diagnostics::StartBatch();
    int files_removed = 0;
    uint64_t bytes_freed = 0;
    
    for (const auto& child : children) {
        // Safety check before every significant operation loop
        if (power::EvaluateIdleState() == power::IdleState::NOT_IDLE) {
            LOGW("Aborting cleanup: device is no longer idle.");
            return false;
        }

        std::string child_path = path + "/" + child;
        struct stat st;
        if (lstat(child_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (!RemoveRecursiveBatched(child_path)) {
                    return false;
                }
            } else {
                uint64_t size = st.st_size;
                if (storage::SafeRemove(child_path)) {
                    files_removed++;
                    bytes_freed += size;
                }
            }
        }
        
        if (files_removed >= config::g_config.max_files_per_batch) {
            if (!diagnostics::EndBatch(files_removed, bytes_freed)) {
                return false; // Limit exceeded
            }
            // Sleep briefly to yield CPU
            usleep(10000); 
            diagnostics::StartBatch();
            files_removed = 0;
            bytes_freed = 0;
        }
    }
    
    diagnostics::EndBatch(files_removed, bytes_freed);
    
    // Finally remove the directory itself
    storage::SafeRemove(path);
    return true;
}

bool CleanDirectorySafely(const CacheDirInfo& target_dir) {
    LOGI("Starting safe clean of: %s", target_dir.path.c_str());
    
    std::vector<std::string> children;
    if (!storage::GetDirectoryChildren(target_dir.path, children)) {
        return true; 
    }
    
    for (const auto& child : children) {
        std::string child_path = target_dir.path + "/" + child;
        if (!RemoveRecursiveBatched(child_path)) {
            LOGW("Cleanup aborted for: %s", target_dir.path.c_str());
            return false;
        }
    }
    
    LOGI("Completed clean of: %s", target_dir.path.c_str());
    
    // Update state after clean
    PackageState state = GetPackageState(target_dir.pkg_name);
    state.last_cleanup_ms = GetCurrentTimeMs();
    state.cleanup_count++;
    state.last_size_bytes = 0; // Cache is now presumably empty
    UpdatePackageState(target_dir.pkg_name, state);
    SaveStateAtomic();
    
    return true;
}

} // namespace cache
} // namespace freshcore
