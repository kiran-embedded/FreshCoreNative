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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#ifndef FITRIM
struct fstrim_range {
    uint64_t start;
    uint64_t len;
    uint64_t minlen;
};
#define FITRIM		_IOWR('X', 121, struct fstrim_range)
#endif

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

static void NativeFstrim(const char* path) {
    if (power::EvaluateIdleState() == power::IdleState::NOT_IDLE) return;
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) return;
    
    struct fstrim_range range;
    memset(&range, 0, sizeof(range));
    range.len = (uint64_t)-1;
    
    LOGI("Running native fstrim on %s...", path);
    if (ioctl(fd, FITRIM, &range) == 0) {
        LOGI("fstrim completed on %s: %llu bytes trimmed", path, (unsigned long long)range.len);
    }
    close(fd);
}

void RunSystemMaintenanceSweeps() {
    LOGI("Starting native system maintenance sweeps...");
    
    // 1. FSTRIM
    NativeFstrim("/data");
    NativeFstrim("/cache");
    
    // 2. Crash logs and temp files
    const char* paths_to_sweep[] = {
        "/data/tombstones",
        "/data/system/dropbox",
        "/data/local/tmp" // Note: we should avoid deleting our own log, but RemoveRecursiveBatched handles children.
    };
    
    for (const char* path : paths_to_sweep) {
        if (power::EvaluateIdleState() == power::IdleState::NOT_IDLE) {
            LOGW("System sweeps aborted due to wake up.");
            return;
        }
        
        std::vector<std::string> children;
        if (storage::GetDirectoryChildren(path, children)) {
            for (const auto& child : children) {
                // Keep our own log!
                if (child.find("freshcore.log") != std::string::npos) continue;
                
                std::string child_path = std::string(path) + "/" + child;
                RemoveRecursiveBatched(child_path);
            }
        }
    }
    LOGI("System maintenance sweeps completed.");
}

} // namespace cache
} // namespace freshcore
