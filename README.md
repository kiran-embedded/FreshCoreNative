# FreshCore Native

[![Android API](https://img.shields.io/badge/API-30%2B-brightgreen.svg?style=flat)](https://android.com)
[![Architecture](https://img.shields.io/badge/Arch-arm64--v8a-blue)](https://android.com)
[![KernelSU](https://img.shields.io/badge/KernelSU-Module-orange)](https://kernelsu.org)

FreshCore is a native C++ Android background service designed to manage application caches. It runs as a KernelSU module.

Rather than running constantly or using aggressive memory-clearing tactics, FreshCore stays suspended using Linux `epoll` and `timerfd`. It only wakes up under specific idle conditions and cleans caches incrementally to prevent device stutter.

## Features

- **Native Implementation**: Written in C++20. Runs as a lightweight native executable without relying on Java or Kotlin background services.
- **Strict Idle Enforcement**: Halts all I/O operations instantly if the screen turns on or if user interaction is detected.
- **Hardware Telemetry**: Monitors thermal zones, battery level, and memory pressure (`/proc/pressure/memory`) before starting any maintenance.
- **Incremental Processing**: Scans and deletes files in bounded batches (e.g., 128 files at a time) to prevent heavy CPU and storage I/O spikes.
- **State Tracking**: Keeps a binary state file to track cache growth rates, preventing unnecessary rescans of packages that haven't changed.

## Installation

1. Download the `FreshCore-KSU.zip` release from the [Releases](https://github.com/kiran-embedded/FreshCoreNative/releases) page.
2. Flash it via the KernelSU Manager app.
3. Reboot your device.

## Uninstallation

Simply remove the module in KernelSU and reboot. The included uninstall script will automatically clean up the configuration directory, leaving no permanent traces on your device.

## Building from Source

You will need CMake and the Android NDK (r26b or newer).

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-30 \
  -DCMAKE_BUILD_TYPE=Release
make
```
