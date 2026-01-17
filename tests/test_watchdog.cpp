
#include <gtest/gtest.h>
#include "SystemMonitor.h"
#include <vector>
#include <cstring>

TEST(WatchdogTest, DetectsMemoryLimitExceeded) {
    SystemMonitor monitor;
    
    // Get current usage
    double initial_usage = monitor.GetMemoryUsage();
    
    // Set a limit lower than current + allocation
    // Assuming we allocate 100MB
    double limit = initial_usage + 50.0; 
    
    // Allocate 100MB
    size_t alloc_size = 100 * 1024 * 1024;
    void* ptr = malloc(alloc_size);
    ASSERT_NE(ptr, nullptr);
    // Touch memory to ensure RSS increase
    memset(ptr, 0, alloc_size);
    
    // Check if limit is exceeded
    // Note: RSS update might not be instantaneous in /proc, but usually fast enough for test
    // We might need a small sleep in a real flaky environment, but let's try direct first.
    
    bool triggered = monitor.CheckMemoryLimit(limit);
    
    // Free memory
    free(ptr);
    
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
