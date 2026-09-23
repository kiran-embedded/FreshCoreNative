#include "package_scanner.hpp"
#include "cache_policy.hpp"
#include "state_storage.hpp"
#include "../storage/filesystem.hpp"
#include "../logging/logger.hpp"
#include <sys/stat.h>
#include <dirent.h>

namespace freshcore {
namespace cache {

static unsigned long long GetDirectorySize(const std::string& dir_path) {
    unsigned long long size = 0;
    std::vector<std::string> children;
    
    if (!storage::GetDirectoryChildren(dir_path, children)) {
        return 0;
    }
    
    for (const auto& child : children) {
        std::string child_path = dir_path + "/" + child;
        struct stat st;
        if (lstat(child_path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                size += GetDirectorySize(child_path);
            } else {
                size += st.st_size;
            }
        }
    }
    return size;
}

std::vector<CacheDirInfo> ScanPackageCache(const android::PackageInfo& pkg) {
    std::vector<CacheDirInfo> cache_dirs;
    
    // Look for typical cache dirs
    std::vector<std::string> candidates = {
        pkg.data_dir + "/cache",
        pkg.data_dir + "/code_cache"
    };
    
    for (const auto& cand : candidates) {
        if (IsValidCachePath(cand, pkg.data_dir)) {
            struct stat st;
            // Stage 1: Just check if it exists and is a directory
            if (lstat(cand.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                CacheDirInfo info;
                info.path = cand;
                info.pkg_name = pkg.name;
                info.size_calculated = false;
                info.size_bytes = 0;
                cache_dirs.push_back(info);
            }
        }
    }
    
    return cache_dirs;
}

void CalculateCacheSize(CacheDirInfo& info) {
    if (!info.size_calculated) {
        info.size_bytes = GetDirectorySize(info.path);
        info.size_calculated = true;
    }
}

} // namespace cache
} // namespace freshcore
