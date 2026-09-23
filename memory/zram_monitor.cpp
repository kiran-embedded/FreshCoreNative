#include "zram_monitor.hpp"
#include <fstream>

namespace freshcore {
namespace memory {

ZramStats GetZramStats() {
    ZramStats stats;
    
    // Check /sys/block/zram0/mm_stat
    std::ifstream mm_stat("/sys/block/zram0/mm_stat");
    if (mm_stat.is_open()) {
        uint64_t orig_data, compr_data, mem_used;
        if (mm_stat >> orig_data >> compr_data >> mem_used) {
            stats.orig_data_size = orig_data;
            stats.mem_used_total = mem_used;
        }
    }
    
    return stats;
}

} // namespace memory
} // namespace freshcore
