
#include <gtest/gtest.h>
#include "SystemMonitor.h"
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>

TEST(WatchdogTest, DetectsMemoryLimitExceeded) {
    SystemMonitor monitor;
    
    // Get current usage
    double initial_usage = monitor.GetMemoryUsage();
    
    // Set a limit lower than current + allocation
    // Assuming we allocate 100MB
    double limit = initial_usage + 50.0; 
    
    // Allocate 100MB using vector to ensure RAII
    size_t alloc_mb = 100;
    size_t alloc_size = alloc_mb * 1024 * 1024;
    std::vector<char> data(alloc_size);
    
    // Force physical allocation (touch pages)
    // Use volatile to prevent compiler optimization
    volatile char* v_ptr = data.data();
    for (size_t i = 0; i < alloc_size; i += 4096) {
        v_ptr[i] = 1;
    }
    
    // Check if limit is exceeded
    // Note: RSS update might not be instantaneous in /proc, but usually fast enough for test
    // We add a retry loop to be robust against lazy OS accounting
    bool triggered = false;
    for (int i = 0; i < 50; ++i) { // Increase retries to 5s
        if (monitor.CheckMemoryLimit(limit)) {
            triggered = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Vector will be freed automatically
    
    EXPECT_TRUE(triggered) << "Watchdog should trigger when usage " << monitor.GetMemoryUsage() << " > limit " << limit;
}

TEST(WatchdogTest, NoTriggerUnderLimit) {
    SystemMonitor monitor;
    double current = monitor.GetMemoryUsage();
    // specific large limit
    EXPECT_FALSE(monitor.CheckMemoryLimit(current + 500.0));
}

TEST(WatchdogTest, NoTriggerZeroLimit) {
    SystemMonitor monitor;
    EXPECT_FALSE(monitor.CheckMemoryLimit(0));
    EXPECT_FALSE(monitor.CheckMemoryLimit(-1));
}
