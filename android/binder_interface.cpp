#include "binder_interface.hpp"

// We will stub this out to avoid NDK binder dependencies for now, 
// as native Binder C++ APIs change dramatically between Android versions.
// Calling service manager from NDK without the framework is possible but fragile.

namespace freshcore {
namespace android {

void InitBinder() {
    // Stub
}

bool IsDeviceInteractive() {
    // Stub: rely on power_state.cpp instead
    return true; 
}

} // namespace android
} // namespace freshcore
