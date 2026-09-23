#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace freshcore {
namespace cache {

struct PackageState {
    uint32_t hash;
    uint64_t last_scan_ms;
    uint64_t last_cleanup_ms;
    uint64_t last_size_bytes;
    uint32_t cleanup_count;
    uint32_t growth_rate_bytes_per_day;
};

// Generates a simple hash for the package name
uint32_t HashPackageName(const std::string& pkg_name);

// Initialize state subsystem (loads state from disk)
void InitializeStateStorage();

// Get the state for a package
PackageState GetPackageState(const std::string& pkg_name);

// Update and persist state
void UpdatePackageState(const std::string& pkg_name, const PackageState& state);
void SaveStateAtomic();

} // namespace cache
} // namespace freshcore
