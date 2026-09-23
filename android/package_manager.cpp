#include "package_manager.hpp"
#include <fstream>
#include <sstream>

namespace freshcore {
namespace android {

std::vector<PackageInfo> GetInstalledPackages() {
    std::vector<PackageInfo> packages;
    
    // /data/system/packages.list format:
    // pkgName uid debuggable dataDir seinfo group [...]
    std::ifstream list_file("/data/system/packages.list");
    if (!list_file.is_open()) return packages;
    
    std::string line;
    while (std::getline(list_file, line)) {
        std::istringstream iss(line);
        PackageInfo info;
        std::string debuggable, seinfo, group;
        
        if (iss >> info.name >> info.uid >> debuggable >> info.data_dir) {
            // Very rough heuristic for system apps in packages.list: uid < 10000
            // Realistically, apps are >= 10000
            info.is_system = (info.uid < 10000);
            packages.push_back(info);
        }
    }
    
    return packages;
}

} // namespace android
} // namespace freshcore
