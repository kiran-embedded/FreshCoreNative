#include "../config/config.hpp"
#include <iostream>
#include <cassert>

using namespace freshcore;

int main() {
    // Simple verification that config parser creates default correctly when file is missing
    config::LoadConfig();
    assert(config::g_config.max_files_per_batch == 128);
    assert(config::g_config.max_batch_time_ms == 250);
    assert(config::g_config.max_total_clean_mb_per_run == 1024);
    
    std::cout << "Config tests passed.\n";
    return 0;
}
