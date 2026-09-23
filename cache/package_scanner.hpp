#pragma once
#include <string>
#include <vector>
#include "../android/package_manager.hpp"

namespace freshcore {
namespace cache {

struct CacheDirInfo {
    std::string path;
    std::string pkg_name;
    unsigned long long size_bytes = 0;
    bool size_calculated = false; // For two-stage scanning
};

// Scan a package's data directory for cache directories
std::vector<CacheDirInfo> ScanPackageCache(const android::PackageInfo& pkg);

// Calculates the size of a cache directory (Stage 2)
void CalculateCacheSize(CacheDirInfo& info);

} // namespace cache
} // namespace freshcore
