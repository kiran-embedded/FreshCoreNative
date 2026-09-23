#pragma once

namespace freshcore {
namespace memory {

// Gets the available memory in KB (MemAvailable from /proc/meminfo)
unsigned long long GetAvailableMemoryKB();

} // namespace memory
} // namespace freshcore
