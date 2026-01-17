#include <gtest/gtest.h>
#include "SystemMonitor.h"
#include <thread>
#include <vector>
#include <cmath>

// 辅助函数：在测试中制造一些 CPU 负载
void burn_cpu(int ms) {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now() - start).count() < ms) {
        // 空循环
        double x = 0.0;
        for(int i=0; i<1000; ++i) x += std::sin(i);
    }
}

// 辅助函数：制造内存负载
void burn_memory(std::vector<char>& buffer, size_t size_mb) {
    buffer.resize(size_mb * 1024 * 1024, 1); // 分配内存并填入数据
}

TEST(SystemMonitorTest, MemoryUsageIsReasonable) {
    SystemMonitor monitor;
    
    // 1. 初始读数应该大于 0
    double initial_mem = monitor.GetMemoryUsage();
    EXPECT_GT(initial_mem, 0.0);
    std::cout << "[Info] Initial Memory: " << initial_mem << " MB" << std::endl;

    // 2. 分配 50MB 内存
    std::vector<char> dummy;
    burn_memory(dummy, 50);

    // 3. 再次读取，应该增加了约 50MB
    double after_mem = monitor.GetMemoryUsage();
    EXPECT_GT(after_mem, initial_mem + 40.0); // 留一些 buffer，系统可能分配多于请求
    std::cout << "[Info] After Alloc 50MB: " << after_mem << " MB" << std::endl;
}

TEST(SystemMonitorTest, CpuUsageIsDynamic) {
    SystemMonitor monitor;

    // 第一次调用通常返回 0 或不准确，用来初始化基准
    double v1 = monitor.GetCpuUsage();
    
    // 睡 200ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 第二次调用，应该反映休息时的情况（接近 0，但不一定是 0，可能有后台进程）
    double v2 = monitor.GetCpuUsage();
    std::cout << "[Info] Idle CPU Usage: " << v2 << "%" << std::endl;
    EXPECT_GE(v2, 0.0);
    EXPECT_LE(v2, 100.0);
}
