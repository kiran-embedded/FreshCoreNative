#pragma once
#include <cstdint>

namespace freshcore {
namespace memory {

struct ZramStats {
    uint64_t mem_used_total = 0; // Bytes used by zram
    uint64_t orig_data_size = 0; // Uncompressed size
};

ZramStats GetZramStats();

} // namespace memory
} // namespace freshcore
