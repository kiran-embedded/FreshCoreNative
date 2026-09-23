#include "state_storage.hpp"
#include "../logging/logger.hpp"
#include <fstream>
#include <vector>
#include <cstdio>
#include <unistd.h>

namespace freshcore {
namespace cache {

static const char* STATE_FILE = "/data/adb/freshcore/state.bin";
static const char* STATE_FILE_TMP = "/data/adb/freshcore/state.tmp";
static const uint32_t STATE_MAGIC = 0x46525348; // "FRSH"
static const uint32_t STATE_VERSION = 1;

struct StateHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t timestamp;
    uint32_t entry_count;
    uint32_t checksum; // simple sum of all bytes after header
};

static std::unordered_map<uint32_t, PackageState> g_states;

uint32_t HashPackageName(const std::string& pkg_name) {
    uint32_t hash = 2166136261u;
    for (char c : pkg_name) {
        hash ^= (uint8_t)c;
        hash *= 16777619;
    }
    return hash;
}

static uint32_t CalculateChecksum(const std::vector<PackageState>& states) {
    uint32_t sum = 0;
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(states.data());
    size_t size = states.size() * sizeof(PackageState);
    for (size_t i = 0; i < size; ++i) {
        sum += ptr[i];
    }
    return sum;
}

void InitializeStateStorage() {
    std::ifstream file(STATE_FILE, std::ios::binary);
    if (!file.is_open()) {
        LOGD("No existing state file found.");
        return;
    }

    StateHeader header;
    if (!file.read(reinterpret_cast<char*>(&header), sizeof(StateHeader))) return;

    if (header.magic != STATE_MAGIC || header.version != STATE_VERSION) {
        LOGW("State file magic/version mismatch, ignoring.");
        return;
    }

    std::vector<PackageState> loaded_states(header.entry_count);
    if (!file.read(reinterpret_cast<char*>(loaded_states.data()), header.entry_count * sizeof(PackageState))) {
        LOGW("State file truncated.");
        return;
    }

    if (CalculateChecksum(loaded_states) != header.checksum) {
        LOGE("State file checksum mismatch! Corruption detected.");
        return;
    }

    for (const auto& st : loaded_states) {
        g_states[st.hash] = st;
    }
    LOGI("Loaded %u package states.", header.entry_count);
}

PackageState GetPackageState(const std::string& pkg_name) {
    uint32_t hash = HashPackageName(pkg_name);
    auto it = g_states.find(hash);
    if (it != g_states.end()) {
        return it->second;
    }
    
    PackageState default_state{};
    default_state.hash = hash;
    return default_state;
}

void UpdatePackageState(const std::string& pkg_name, const PackageState& state) {
    uint32_t hash = HashPackageName(pkg_name);
    g_states[hash] = state;
}

void SaveStateAtomic() {
    std::vector<PackageState> state_vec;
    state_vec.reserve(g_states.size());
    for (const auto& pair : g_states) {
        state_vec.push_back(pair.second);
    }

    StateHeader header;
    header.magic = STATE_MAGIC;
    header.version = STATE_VERSION;
    
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    header.timestamp = (uint64_t)ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
    
    header.entry_count = state_vec.size();
    header.checksum = CalculateChecksum(state_vec);

    FILE* f = fopen(STATE_FILE_TMP, "wb");
    if (!f) {
        LOGE("Failed to open %s for writing.", STATE_FILE_TMP);
        return;
    }

    fwrite(&header, sizeof(StateHeader), 1, f);
    fwrite(state_vec.data(), sizeof(PackageState), state_vec.size(), f);
    
    fflush(f);
    fsync(fileno(f));
    fclose(f);

    if (rename(STATE_FILE_TMP, STATE_FILE) != 0) {
        LOGE("Failed to rename temporary state file.");
    }
}

} // namespace cache
} // namespace freshcore
