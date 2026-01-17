#include "SystemMonitor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

double SystemMonitor::GetMemoryUsage() {
    // 打开当前进程的状态文件 /proc/self/stat
    std::ifstream stat_file("/proc/self/stat");
    if (!stat_file.is_open()) {
        return 0.0;
    }

    std::string field;
    // /proc/[pid]/stat 文件中第 24 个字段是 rss (Resident Set Size)
    // 它表示进程当前占用的物理内存页数。我们需要跳过前 23 个字段。
    for (int i = 0; i < 24; ++i) {
        if (!(stat_file >> field)) {
            return 0.0;
        }
    }

    // 将第 24 个字段（rss 页数）转换为长整型
    long rss = std::stol(field);
    // 获取系统的内存页大小（通过 sysconf 获取，通常为 4096 字节）
    long page_size = sysconf(_SC_PAGESIZE);
    
    // 计算总内存使用量（字节）并转换为 MB
    // 计算公式：(页数 * 每页字节数) / (1024 * 1024)
    return (static_cast<double>(rss) * page_size) / (1024.0 * 1024.0);
}

double SystemMonitor::GetCpuUsage() {
    // 加锁，防止多线程同时修改 last_total_cpu_time_ 等状态
    std::lock_guard<std::mutex> lock(mutex_);

    // 1. 打开 /proc/stat 文件，这是全局 CPU 状态
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) {
        return 0.0;
    }

    std::string line;
    std::getline(stat_file, line); // 读取第一行，通常以 "cpu " 开头，代表总 CPU

    std::istringstream iss(line);
    std::string cpu_label;
    iss >> cpu_label; // 跳过 "cpu" 字符串

    // 2. 读取各个时间片数据
    // Linux 内核文档定义了这些字段顺序：user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
    int64_t user, nice, system, idle, iowait, irq, softirq, steal;
    
    // 只读取前 8 个字段通常足够计算
    if (!(iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)) {
        return 0.0;
    }

    // 3. 计算 Total 和 Idle
    // Idle 时间 = idle + iowait
    // Total 时间 = user + nice + system + idle + iowait + irq + softirq + steal
    int64_t current_idle = idle + iowait;
    int64_t current_total = user + nice + system + idle + iowait + irq + softirq + steal;

    // 4. 计算与上一次读数的差值 (Delta)
    int64_t total_delta = current_total - last_total_cpu_time_;
    int64_t idle_delta = current_idle - last_idle_cpu_time_;

    // 5. 更新状态以备下次使用
    last_total_cpu_time_ = current_total;
    last_idle_cpu_time_ = current_idle;

    // 防止除以零（通常也是处理第一次运行的情况）
    if (total_delta <= 0) {
        return 0.0;
    }

    // 6. 计算使用率
    // 这里的逻辑反向思考：使用率 = 1.0 - 空闲率
    double usage = 100.0 * (1.0 - static_cast<double>(idle_delta) / static_cast<double>(total_delta));
    return usage < 0 ? 0 : usage;
}
