#include "mic_fixer.hpp"
#include "../logging/logger.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <sys/resource.h>

namespace freshcore {
namespace mic_fixer {

static std::atomic<bool> g_running{false};
static std::thread g_poll_thread;
static std::string g_tinymix_path = "tinymix"; // Default

static std::string ExecCommand(const char* cmd) {
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

static bool RunTinymix(const std::string& control, const std::string& val1, const std::string& val2 = "", const std::string& val3 = "") {
    std::string cmd = g_tinymix_path + " \"" + control + "\" " + val1;
    if (!val2.empty()) cmd += " " + val2;
    if (!val3.empty()) cmd += " " + val3;
    
    // Capture both stdout and stderr
    cmd += " 2>&1";
    
    std::string output = ExecCommand(cmd.c_str());
    
    if (output.find("Invalid") != std::string::npos || output.find("Could not") != std::string::npos || output.find("usage:") != std::string::npos) {
        LOGW("X FAILED: '%s' is not supported on this device's audio chip.", control.c_str());
        return false;
    } else {
        LOGI("V SUCCESS: Applied '%s' flawlessly.", control.c_str());
        return true;
    }
}

static void ApplyPreInit() {
    // Wake up the hardware BEFORE the user hits answer!
    // FIX: Actively MUTE the transmission line first so the receiver doesn't hear the buggy default route's echo.
    RunTinymix("Voice Tx Mute", "1", "1", "1");
    RunTinymix("Voip Tx Mute", "1", "1");
    
    RunTinymix("TX_AIF1_CAP Mixer DEC0", "1");
    RunTinymix("TX_AIF1_CAP Mixer DEC1", "1");
    RunTinymix("ADC1_MIXER Switch", "1");
    RunTinymix("ADC2_MIXER Switch", "1");
    
    RunTinymix("ADC1 MUX", "INP2");
    RunTinymix("ADC2 MUX", "INP3");

    RunTinymix("IIR0 INP0 MUX", "DEC0");
}

static void ApplyMicFix() {
    // Hardware-specific Mic Routing
    RunTinymix("TX_CDC_DMA_TX_3 Channels", "Two");
    
    RunTinymix("TX DEC0 MUX", "SWR_MIC");
    RunTinymix("TX DEC1 MUX", "SWR_MIC");

    RunTinymix("TX SMIC MUX0", "ADC0");
    RunTinymix("TX SMIC MUX1", "ADC2");

    RunTinymix("IIR0 INP0 MUX", "DEC0");

    RunTinymix("RX_RX0 Digital Volume", "84");
    RunTinymix("TX_DEC0 Volume", "84");
    RunTinymix("TX_DEC1 Volume", "84");
    
    // FIX: Now that the hardware routes are completely rebuilt and safe, UNMUTE the transmission line.
    // This replaces the millisecond echo with absolute silence.
    RunTinymix("Voice Tx Mute", "0", "0", "0");
    RunTinymix("Voip Tx Mute", "0", "0");
}

static void ShowNotification() {
    std::system("cmd notification post -S bigtext -t \"FreshCore MicFix Active\" \"FreshCore\" \"Audio Chip Fully Reinitialized!\" >/dev/null 2>&1 &");
}

static void PollLoop() {
    // Drop thread priority to absolute lowest to completely eliminate Call UI lag!
    setpriority(PRIO_PROCESS, 0, 19);
    
    // Discover tinymix path
    std::string path_check = ExecCommand("command -v tinymix 2>/dev/null");
    if (!path_check.empty() && path_check.find("tinymix") != std::string::npos) {
        path_check.erase(path_check.find_last_not_of(" \n\r\t") + 1);
        g_tinymix_path = path_check;
    } else {
        LOGW("tinymix not found in PATH! Using default.");
    }
    LOGI("MicFix Engine started. Polling call state reliably at low priority.");

    bool active = false;
    bool pre_init = false;
    
    while (g_running) {
        // Use timeout to prevent hanging if AudioService is deadlocked
        std::string state = ExecCommand("timeout 2 dumpsys audio 2>/dev/null | grep -m 1 -i 'mMode='");
        
        if (state.find("MODE_RINGTONE") != std::string::npos || state.find("mMode=1") != std::string::npos) {
            if (!pre_init) {
                pre_init = true;
                LOGI("RINGTONE DETECTED! Powering on Audio Hardware early...");
                ApplyPreInit();
            }
        } 
        else if (state.find("MODE_IN_CALL") != std::string::npos || 
                 state.find("MODE_IN_COMMUNICATION") != std::string::npos || 
                 state.find("mMode=2") != std::string::npos || 
                 state.find("mMode=3") != std::string::npos) {
            if (!active) {
                active = true;
                LOGI("CALL CONNECTED! Aggressively fully re-initializing Handset Mic route!");
                ShowNotification();
                ApplyPreInit();
                ApplyMicFix();
                LOGI("Hardware Primed and Ready!");
            }
        } 
        else {
            if (active || pre_init) {
                active = false;
                pre_init = false;
                LOGI("Call ended. Audio routes returning to idle state.");
            }
        }

        // Sleep safely. 3 seconds avoids AudioService lock contention.
        for (int i = 0; i < 30 && g_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

// ==========================================
// ALSA INOTIFY HYBRID ENGINE
// ==========================================

#include <sys/inotify.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>

static void AlsaInotifyLoop(int fd) {
    LOGI("Native ALSA Hooking Active! (0% CPU Event-Driven Mode)");
    
    // Discover tinymix path
    std::string path_check = ExecCommand("command -v tinymix 2>/dev/null");
    if (!path_check.empty() && path_check.find("tinymix") != std::string::npos) {
        path_check.erase(path_check.find_last_not_of(" \n\r\t") + 1);
        g_tinymix_path = path_check;
    }

    char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    bool active = false;

    while (g_running) {
        struct pollfd pfd = { fd, POLLIN, 0 };
        // Use poll with 500ms timeout so we can check g_running to shut down gracefully
        int ret = poll(&pfd, 1, 500); 

        if (ret < 0) {
            break;
        } else if (ret == 0) {
            continue; // timeout, check g_running again
        }

        ssize_t len = read(fd, buffer, sizeof(buffer));
        if (len <= 0) break;

        const struct inotify_event *event;
        for (char *ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event->len) {
            event = (const struct inotify_event *) ptr;

            if (event->mask & IN_OPEN) {
                active = true;
                LOGI("ALSA INTERCEPT: Capture Stream Opened! (0 ms delay)");
                // Forcefully apply the fix every single time a stream opens.
                // This guarantees Android can never overwrite it without us knowing.
                ApplyPreInit();
                ApplyMicFix();
                ShowNotification();
                LOGI("Hardware Primed and Ready natively!");
            }
            if (event->mask & (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE)) {
                if (active) {
                    active = false;
                    LOGI("ALSA INTERCEPT: Capture Stream Closed.");
                }
            }
        }
    }
    close(fd);
}

void StartPolling() {
    if (g_running) return;
    g_running = true;

    // Attempt to initialize ALSA Hooking first
    int fd = inotify_init1(IN_NONBLOCK);
    bool alsa_success = false;

    if (fd >= 0) {
        DIR *dir = opendir("/dev/snd");
        if (dir) {
            struct dirent *ent;
            while ((ent = readdir(dir)) != nullptr) {
                std::string filename = ent->d_name;
                // Only watch 'capture' devices (e.g., pcmC0D0c)
                if (filename.back() == 'c' && filename.find("pcm") == 0) {
                    std::string full_path = std::string("/dev/snd/") + filename;
                    if (inotify_add_watch(fd, full_path.c_str(), IN_OPEN | IN_CLOSE) >= 0) {
                        alsa_success = true;
                    }
                }
            }
            closedir(dir);
        }
    }

    if (alsa_success) {
        g_poll_thread = std::thread(AlsaInotifyLoop, fd);
    } else {
        if (fd >= 0) close(fd);
        LOGW("Native ALSA Hooking unavailable. Falling back to Dumpsys polling loop.");
        g_poll_thread = std::thread(PollLoop);
    }
}

void StopPolling() {
    if (!g_running) return;
    g_running = false;
    if (g_poll_thread.joinable()) {
        g_poll_thread.join();
    }
}

} // namespace mic_fixer
} // namespace freshcore
