#include "memory_monitor.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace memory {

unsigned long long GetAvailableMemoryKB() {
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.compare(0, 13, "MemAvailable:") == 0) {
            unsigned long long kb = 0;
            if (sscanf(line.c_str(), "MemAvailable: %llu kB", &kb) == 1) {
                return kb;
            }
        }
    }
    return 0;
}

} // namespace memory
} // namespace freshcore
