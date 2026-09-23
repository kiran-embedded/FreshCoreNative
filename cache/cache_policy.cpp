#include "cache_policy.hpp"
#include "../config/config.hpp"
#include "../storage/filesystem.hpp"

namespace freshcore {
namespace cache {

bool IsValidCachePath(const std::string& path, const std::string& package_data_dir) {
    if (!storage::IsPathUnderRoot(path, package_data_dir)) {
        return false;
    }
    
    std::string canon = storage::GetCanonicalPath(path);
    if (canon.empty()) return false;
    
    // Only allow specific known cache directories
    // /data/user/0/<pkg>/cache
    // /data/user/0/<pkg>/code_cache
    
    size_t last_slash = canon.find_last_of('/');
    if (last_slash == std::string::npos) return false;
    
    std::string dir_name = canon.substr(last_slash + 1);
    if (dir_name == "cache" || dir_name == "code_cache") {
        return true;
    }
    
    return false;
}

bool ShouldCleanCache(unsigned long long cache_size_bytes) {
    unsigned long long threshold_bytes = (unsigned long long)config::g_config.cache_threshold_mb * 1024 * 1024;
    return cache_size_bytes >= threshold_bytes;
}

} // namespace cache
} // namespace freshcore
