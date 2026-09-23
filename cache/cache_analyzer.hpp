#pragma once
#include "package_scanner.hpp"
#include <vector>

namespace freshcore {
namespace cache {

struct MaintenanceTarget {
    CacheDirInfo cache_dir;
    int priority = 0;
};

// Analyzes scanned cache dirs and returns a prioritized list of targets
// Takes a mutable reference because it will perform Stage 2 scanning.
std::vector<MaintenanceTarget> AnalyzeCaches(std::vector<CacheDirInfo>& scanned_dirs);

} // namespace cache
} // namespace freshcore
