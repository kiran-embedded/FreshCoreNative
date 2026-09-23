cmake_minimum_required(VERSION 3.22.1)
project(FreshCoreNative)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aggressive optimizations for release
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -fno-exceptions -fno-rtti -fvisibility=hidden -fvisibility-inlines-hidden")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -Wl,--gc-sections -Wl,-s")
endif()

add_compile_options(-Wall -Wextra -Werror)

# Include directories
include_directories(.)

# Source files
set(SOURCES
    core/main.cpp
    core/scheduler.cpp
    core/decision_engine.cpp
    core/state.cpp
    android/binder_interface.cpp
    android/package_manager.cpp
    android/power_state.cpp
    android/activity_state.cpp
    cache/package_scanner.cpp
    cache/cache_analyzer.cpp
    cache/cache_cleaner.cpp
    cache/cache_policy.cpp
    cache/state_storage.cpp
    memory/memory_monitor.cpp
    memory/psi_monitor.cpp
    memory/zram_monitor.cpp
    power/battery_monitor.cpp
    power/thermal_monitor.cpp
    power/idle_detector.cpp
    storage/storage_monitor.cpp
    storage/filesystem.cpp
    logging/logger.cpp
    config/config.cpp
    diagnostics/diagnostics.cpp
)

add_executable(freshcore ${SOURCES})
target_compile_options(freshcore PRIVATE -O2 -fvisibility=hidden -DNDEBUG)

option(BUILD_TESTING "Build the testing tree" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Need log library for android if we eventually fall back to it, but we use our own logger.
target_link_libraries(freshcore log)
