#pragma once
#include <string>

namespace freshcore {
namespace android {

// Check if a specific package is currently in the foreground
bool IsPackageForeground(const std::string& package_name);

// Check if ANY heavy foreground activity (gaming, camera) is ongoing
bool IsHeavyForegroundActivity();

} // namespace android
} // namespace freshcore
