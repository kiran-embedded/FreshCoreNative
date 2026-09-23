#include "storage_monitor.hpp"
#include <sys/statvfs.h>

namespace freshcore {
namespace storage {

StorageStats GetStorageStats(const std::string& path) {
    StorageStats stats;
    struct statvfs vfs;
    
    if (statvfs(path.c_str(), &vfs) == 0) {
        stats.total_bytes = (unsigned long long)vfs.f_blocks * vfs.f_frsize;
        stats.free_bytes = (unsigned long long)vfs.f_bavail * vfs.f_frsize;
        if (stats.total_bytes > 0) {
            stats.free_percent = ((double)stats.free_bytes / stats.total_bytes) * 100.0;
        }
    }
    
    return stats;
}

} // namespace storage
} // namespace freshcore
