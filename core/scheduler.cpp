#include "scheduler.hpp"
#include "decision_engine.hpp"
#include "state.hpp"
#include "../config/config.hpp"
#include "../logging/logger.hpp"
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <csignal>

namespace freshcore {
namespace core {

static volatile bool g_running = true;
static int g_epoll_fd = -1;
static int g_timer_fd = -1;

static void SignalHandler(int signum) {
    (void)signum;
    g_running = false;
}

static void SetTimer(int timer_fd, unsigned int seconds) {
    struct itimerspec its;
    its.it_value.tv_sec = seconds;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    timerfd_settime(timer_fd, 0, &its, nullptr);
}

void RunScheduler() {
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    g_epoll_fd = epoll_create1(0);
    if (g_epoll_fd == -1) {
        LOGE("Failed to create epoll instance");
        return;
    }
    
    g_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (g_timer_fd == -1) {
        LOGE("Failed to create timerfd");
        close(g_epoll_fd);
        return;
    }
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = g_timer_fd;
    if (epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, g_timer_fd, &ev) == -1) {
        LOGE("Failed to add timerfd to epoll");
        close(g_timer_fd);
        close(g_epoll_fd);
        return;
    }
    
    LOGI("Scheduler Event Loop started");
    
    // Initial evaluation immediately
    SetTimer(g_timer_fd, 1);
    
    struct epoll_event events[10];
    
    while (g_running) {
        int nfds = epoll_wait(g_epoll_fd, events, 10, -1);
        if (nfds == -1) {
            if (errno == EINTR) continue;
            LOGE("epoll_wait failed");
            break;
        }
        
        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == g_timer_fd) {
                // Read to clear the timer event
                uint64_t expirations;
                if (read(g_timer_fd, &expirations, sizeof(expirations)) == -1) {
                    continue;
                }
                
                // Perform state evaluation
                EvaluateAndTransition();
                
                // Determine next sleep based on state
                EngineState state = GetCurrentState();
                unsigned int next_sleep_sec = 60; // Default fallback
                
                switch (state) {
                    case EngineState::ACTIVE:
                        next_sleep_sec = config::g_config.check_interval_active_sec;
                        break;
                    case EngineState::SCREEN_OFF:
                        next_sleep_sec = config::g_config.check_interval_screen_off_sec;
                        break;
                    case EngineState::IDLE_CANDIDATE:
                        next_sleep_sec = config::g_config.check_interval_deep_idle_sec;
                        break;
                    case EngineState::DEEP_IDLE:
                    case EngineState::MAINTENANCE:
                        // Just returned from maintenance, fall back to checking if idle holds
                        next_sleep_sec = config::g_config.check_interval_screen_off_sec;
                        break;
                    case EngineState::ABORTING:
                        next_sleep_sec = 30; // Brief cooldown after aborting
                        break;
                    case EngineState::ERROR_BACKOFF:
                        next_sleep_sec = 300; // 5 min backoff
                        break;
                    default:
                        next_sleep_sec = 60;
                        break;
                }
                
                SetTimer(g_timer_fd, next_sleep_sec);
            }
        }
    }
    
    close(g_timer_fd);
    close(g_epoll_fd);
    LOGI("Scheduler stopped");
}

} // namespace core
} // namespace freshcore
