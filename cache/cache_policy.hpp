#pragma once
#include <string>

namespace freshcore {
namespace cache {

// Check if a directory path is a valid cache directory we are allowed to clean
bool IsValidCachePath(const std::string& path, const std::string& package_data_dir);

// Check if a specific cache directory size exceeds the configured threshold
bool ShouldCleanCache(unsigned long long cache_size_bytes);

} // namespace cache
} // namespace freshcore
