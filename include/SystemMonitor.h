#pragma once

#include <string>
#include <cstdint>
#include <mutex>

/**
 * @brief 负责监控系统资源（CPU 和 内存）
 * 
 * 这是一个工具类，直接读取 Linux 的 /proc 文件系统。
 */
class SystemMonitor {
public:
    SystemMonitor() = default;
    ~SystemMonitor() = default;

    /**
     * @brief 获取当前进程的物理内存占用 (RSS)
     * @return 占用内存大小，单位：MB
     */
    double GetMemoryUsage();

    /**
     * @brief 获取系统的整体 CPU 使用率
     * 
     * 注意：这是一个瞬时值计算。
     * 由于 /proc/stat 存储的是累加值，我们需要维护上一次的读数，
     * 计算 (Current - Last) 的差值来得出这段时间内的使用率。
     * 
     * @return CPU 使用率百分比 (0.0 - 100.0)
     */
    double GetCpuUsage();

private:
    // 记录上一次读取的 CPU 时间片总和
    int64_t last_total_cpu_time_ = 0;
    // 记录上一次读取的 CPU 空闲时间
    int64_t last_idle_cpu_time_ = 0;
    // 互斥锁，保证多线程访问安全
    std::mutex mutex_;
};
