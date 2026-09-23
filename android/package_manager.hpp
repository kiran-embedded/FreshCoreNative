#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace freshcore {
namespace android {

struct PackageInfo {
    std::string name;
    std::string data_dir;
    int uid = -1;
    bool is_system = false;
};

// Parses /data/system/packages.list to get all installed packages
std::vector<PackageInfo> GetInstalledPackages();

} // namespace android
} // namespace freshcore
