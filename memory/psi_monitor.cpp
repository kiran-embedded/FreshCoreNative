#include "psi_monitor.hpp"
#include "../logging/logger.hpp"
#include <fstream>
#include <string>

namespace freshcore {
namespace memory {

PSIStats GetMemoryPSI() {
    PSIStats stats;
    std::ifstream psi_mem("/proc/pressure/memory");
    if (!psi_mem.is_open()) {
        return stats;
    }

    stats.is_supported = true;
    std::string line;
    while (std::getline(psi_mem, line)) {
        // some avg10=0.00 avg60=0.00 avg300=0.00 total=1234
        if (line.compare(0, 4, "some") == 0) {
            sscanf(line.c_str(), "some avg10=%f", &stats.some_avg10);
        } else if (line.compare(0, 4, "full") == 0) {
            sscanf(line.c_str(), "full avg10=%f", &stats.full_avg10);
        }
    }
    return stats;
}

bool IsMemoryPressureSafe() {
    PSIStats stats = GetMemoryPSI();
    if (!stats.is_supported) {
        return true; // Assume safe if PSI is not supported
    }

    // A heuristic: if 'some' tasks are delayed > 15% of the time over 10s, pressure is high
    if (stats.some_avg10 > 15.0f) {
        LOGD("Memory pressure high (some avg10 = %.2f)", stats.some_avg10);
        return false;
    }
    return true;
}

} // namespace memory
} // namespace freshcore
