#pragma once
#include <string>

namespace freshcore {
namespace android {

// Initialize binder thread pool if needed
void InitBinder();

// Use binder to check if device is interactive (more reliable than sysfs if it works)
bool IsDeviceInteractive();

} // namespace android
} // namespace freshcore
