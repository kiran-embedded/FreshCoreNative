#include "decision_engine.hpp"
#include "state.hpp"
#include "../power/idle_detector.hpp"
#include "../android/power_state.hpp"
#include "../cache/package_scanner.hpp"
#include "../cache/cache_analyzer.hpp"
#include "../cache/cache_cleaner.hpp"
#include "../cache/state_storage.hpp"
#include "../logging/logger.hpp"

namespace freshcore {
namespace core {

static void PerformMaintenance() {
    auto packages = android::GetInstalledPackages();
    std::vector<cache::CacheDirInfo> all_cache_dirs;
    
    for (const auto& pkg : packages) {
        // Abort check
        if (android::IsScreenOn()) {
            TransitionTo(EngineState::ABORTING);
            return;
        }
        
        auto pkg_caches = cache::ScanPackageCache(pkg);
        all_cache_dirs.insert(all_cache_dirs.end(), pkg_caches.begin(), pkg_caches.end());
    }
    
    auto targets = cache::AnalyzeCaches(all_cache_dirs);
    
    for (const auto& target : targets) {
        // Double check idle before cleaning
        if (power::EvaluateIdleState() == power::IdleState::NOT_IDLE) {
            TransitionTo(EngineState::ABORTING);
            return;
        }
        
        if (!cache::CleanDirectorySafely(target.cache_dir)) {
            LOGW("Aborted cleaning on %s", target.cache_dir.path.c_str());
            TransitionTo(EngineState::ABORTING);
            return;
        }
    }
    
    // Finished successfully
    TransitionTo(EngineState::ACTIVE); 
}

void EvaluateAndTransition() {
    EngineState current = GetCurrentState();
    power::IdleState idle_state = power::EvaluateIdleState();

    if (idle_state == power::IdleState::NOT_IDLE) {
        if (current != EngineState::ACTIVE) {
            TransitionTo(EngineState::ACTIVE);
        }
        return;
    }

    switch (current) {
        case EngineState::BOOT:
        case EngineState::ACTIVE:
            TransitionTo(EngineState::SCREEN_OFF);
            break;
            
        case EngineState::SCREEN_OFF:
            if (idle_state == power::IdleState::IDLE_CANDIDATE) {
                TransitionTo(EngineState::IDLE_CANDIDATE);
            }
            break;
            
        case EngineState::IDLE_CANDIDATE:
            // Handled by timerfd now in scheduler
            TransitionTo(EngineState::DEEP_IDLE);
            break;
            
        case EngineState::DEEP_IDLE:
            TransitionTo(EngineState::MAINTENANCE);
            PerformMaintenance();
            break;
            
        case EngineState::MAINTENANCE:
            break;
            
        case EngineState::ABORTING:
            TransitionTo(EngineState::ACTIVE);
            break;
            
        case EngineState::ERROR_BACKOFF:
            TransitionTo(EngineState::ACTIVE);
            break;
    }
}

} // namespace core
} // namespace freshcore
