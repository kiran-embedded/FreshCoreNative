#pragma once
#include "package_scanner.hpp"
#include <string>

namespace freshcore {
namespace cache {

// Cleans a directory in batches, respecting max_files_per_batch and safety checks
// Returns true if completed, false if aborted.
bool CleanDirectorySafely(const CacheDirInfo& target_dir);

} // namespace cache
} // namespace freshcore
