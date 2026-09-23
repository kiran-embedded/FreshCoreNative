#pragma once
#include <string>

namespace freshcore {
namespace storage {

struct StorageStats {
    unsigned long long total_bytes = 0;
    unsigned long long free_bytes = 0;
    double free_percent = 0.0;
};

// Gets stats for a specific mount point, usually "/data"
StorageStats GetStorageStats(const std::string& path = "/data");

} // namespace storage
} // namespace freshcore
