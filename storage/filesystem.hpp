#pragma once
#include <string>
#include <vector>

namespace freshcore {
namespace storage {

// Safely gets absolute, resolved path (resolving symlinks)
std::string GetCanonicalPath(const std::string& path);

// Safely gets children of a directory using openat/fdopendir to prevent TOCTOU
// Returns false on failure.
bool GetDirectoryChildren(const std::string& dir_path, std::vector<std::string>& out_children);

// Safely removes a file or empty directory
bool SafeRemove(const std::string& path);

// Validates that path is strictly under a given root
bool IsPathUnderRoot(const std::string& path, const std::string& root);

} // namespace storage
} // namespace freshcore
