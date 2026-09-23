<div align="center">
  <h1>⚙️ FreshCore Native ⚙️</h1>
  <p><strong>Advanced Android KernelSU Maintenance Engine</strong></p>
  <p>Engineered in pure C++ by <strong>Kiran_embedded</strong></p>

  <a href="https://github.com/kiran-embedded/FreshCoreNative">
    <img src="https://img.shields.io/badge/GitHub-Repository-181717.svg?style=for-the-badge&logo=github">
  </a>
  <img src="https://img.shields.io/badge/Android_API-30%2B-3DDC84.svg?style=for-the-badge&logo=android">
  <img src="https://img.shields.io/badge/Architecture-ARM64-blue.svg?style=for-the-badge">
  <img src="https://img.shields.io/badge/KernelSU-Module-orange.svg?style=for-the-badge">
</div>

---

## 📌 Overview

FreshCore Native is a strict, deterministic background daemon designed for custom ROM environments via KernelSU. It replaces bloated shell scripts with a compiled C++ machine code architecture, guaranteeing 0% background battery drain and completely eliminating UI stutter. 

It provides automated storage optimization, aggressive Doze state management, and a robust hardware-level microphone routing fix.

## 🚀 Key Features

### 1. Universal MicFix
Custom ROMs frequently suffer from broken audio HALs, causing the microphone to fail during standard phone calls or VoIP calls (WhatsApp, Telegram).
- **Passive Monitoring:** FreshCore polls the Android AudioService state natively.
- **Priority Yielding:** The polling thread is strictly assigned to Linux Priority 19 (the absolute lowest background priority). When a call connects, it instantly yields the CPU to the Call UI (Dialer app), completely eliminating screen lag.
- **Hardware Injection:** It fires low-level ALSA (`tinymix`) commands directly into the audio chipset to manually rebuild the microphone routes.

### 2. Extreme Doze Optimization (Deep Sleep)
Android's default Doze mode is notoriously slow to engage and disengage.
- **Instant Suspend:** Once the engine completes a background cache sweep, it immediately forces the kernel into maximum Deep Doze (`dumpsys deviceidle force-idle`), aggressively shutting down background wakelocks to maximize overnight battery life.
- **Instant Wake:** The exact millisecond the display turns on, FreshCore fires an `unforce` command. This instantly snaps the CPU out of Deep Doze, bypassing Android's sluggish wake-up phase and delivering a 100% lag-free lockscreen experience.

### 3. Automated Storage Trimming & Cache Sweeping
Flash storage degrades rapidly with random writes and fragmented blocks.
- **Deterministic Idle Trigger:** FreshCore tracks screen state and invokes a precise 15-minute `timerfd` delay using Linux `epoll`. 
- **Hardware Telemetry:** Before touching the disk, it verifies the battery percentage and thermal zone temperatures via `/sys` nodes to ensure it is safe to perform heavy I/O operations.
- **Physical Trimming:** It executes native `fstrim` on all partitions and drops kernel page caches to free up massive amounts of RAM for intensive tasks like gaming.

### 4. Boot-Time Kernel Tweaks
FreshCore injects direct TCP window scaling parameters into the IPv4 stack (`/proc/sys/net/ipv4/tcp_window_scaling`) for maximized network throughput, and forces physical block read-ahead buffers to 2048 KB for faster app launches.

---

## 🧠 Architectural Design (For Developers)

FreshCore is engineered to fix the fatal flaws of traditional Magisk shell scripts (e.g., constant `while true; sleep;` loops that spawn hundreds of processes and lock the system).

### The C++ Advantage
By utilizing a compiled language and the Linux `epoll` interface, the event loop consumes **zero CPU cycles** while sleeping. There are no sub-shells spawned, no parsing overhead, and no OOM-killer vulnerability. 

### AudioService Lock Avoidance
Traditional MicFix scripts execute heavy commands like `dumpsys audio` in a rapid loop, which places a hard lock on the system's `AudioService`. When an incoming call arrives, the Dialer UI attempts to acquire this lock and deadlocks, causing severe screen lag. FreshCore resolves this by using an isolated, nice-level 19 background thread coupled with spaced intervals, ensuring the UI thread always wins the race condition.

---

## 📋 DeepLevel Logging

FreshCore provides highly detailed, human-readable telemetry logs for debugging and analysis.

**Log Location:** `Internal Storage/Download/FreshCore_Report.txt`

The logging engine tracks:
- System state transitions (Boot -> Active -> Idle)
- Hardware telemetry (Battery %, Temperature °C)
- Individual `tinymix` ALSA route success/failure statuses.

*Note: The engine features an automatic 7-day log rotation mechanism. Logs older than 7 days are automatically purged to prevent storage bloat.*

---

## 📦 Installation & Download

1. Download the latest version: **[`FreshCore-KSU_v1.0.zip`](https://github.com/kiran-embedded/FreshCoreNative/releases)**
2. Open your KernelSU (or Magisk) Manager app.
3. Go to the Modules tab and select **Install from storage**.
4. Select `FreshCore-KSU_v1.0.zip` and wait for the custom flashing animation to complete.
5. Reboot your device.

---

## 📋 DeepLevel Logging & Real-Time Monitoring

FreshCore provides highly detailed, human-readable telemetry logs for debugging and analysis.

**Log Location:** `/Internal Storage/Download/FreshCore_Report.txt`

The logging engine tracks:
- System state transitions (Boot -> Active -> Idle)
- Hardware telemetry (Battery %, Temperature °C)
- Individual `tinymix` ALSA route success/failure statuses.

### 🖥️ Live Monitoring (Termux)
If you are an advanced user and want to watch the C++ engine make decisions in real-time, you can stream the log live using a terminal emulator like Termux:

```bash
su
tail -f /storage/emulated/0/Download/FreshCore_Report.txt
```

*Note: The engine features an automatic 7-day log rotation mechanism. Logs older than 7 days are automatically purged to prevent storage bloat.*

---

## 🛠️ Troubleshooting & Help

### Q: The log file isn't showing up in my Download folder!
**A:** Ensure your device has fully booted. KernelSU waits until the boot process is 100% complete before mounting the `/sdcard` storage to write the log safely. If it still doesn't appear, ensure you are running Android 11 (API 30) or newer.

### Q: I see `FAILED: not supported` in my logs for some `tinymix` commands.
**A:** This is 100% intentional and safe! Different devices route microphones through different audio channels (e.g., `ADC1` vs `ADC2`). FreshCore is designed to be **Universal**, so it tries both. Your phone's audio chip will simply reject the channel it doesn't have, and FreshCore will safely skip it and apply the correct one.

### Q: How do I know the background engine is working?
**A:** When you wake your device, check the `FreshCore_Report.txt` log. You should immediately see a line stating: `Waking system from Doze to prevent lockscreen lag...`. This confirms the engine successfully fired the `dumpsys unforce` command the exact millisecond you touched your screen.

### Q: How do I uninstall FreshCore?
**A:** Simply remove or disable the module inside the KernelSU app and reboot. The included `uninstall.sh` script will automatically clean up all cache records, configurations, and binaries, leaving your device completely untouched.

---

## ⚙️ Build Instructions

To compile the C++ source yourself, you require CMake and the Android NDK (r26b+).

```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-30 \
  -DCMAKE_BUILD_TYPE=Release
make
```

---
<div align="center">
  <p>Engineered for stability. Optimized for performance.</p>
</div>
