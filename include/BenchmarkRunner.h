#pragma once

#include "InferenceEngine.h"
#include "SystemMonitor.h"
#include <vector>
#include <cstdint>

/**
 * @brief 压测配置参数
 */
struct BenchmarkConfig {
    int threads = 1;        ///< 并发线程数
    int requests = 100;     ///< 总请求数
    int warmup_rounds = 10; ///< 预热轮数（不计入统计）
};

/**
 * @brief 压测结果统计
 */
struct BenchmarkResult {
    double qps = 0.0;            ///< 吞吐量 (Queries Per Second)
    double avg_latency_ms = 0.0; ///< 平均延迟 (毫秒)
    double p99_latency_ms = 0.0; ///< P99 延迟 (毫秒)
    double avg_cpu_usage = 0.0;  ///< 平均 CPU 使用率 (%)
    double peak_memory_mb = 0.0; ///< 峰值内存占用 (MB)
};

/**
 * @brief 压测核心调度器
 * 
 * 负责管理线程池、分发推理任务、收集性能数据以及控制系统监控。
 * 采用“抢单模式”进行负载均衡。
 */
class BenchmarkRunner {
public:
    /**
     * @brief 构造函数
     * 
     * @param engine 依赖的推理引擎 (引用，生命周期需长于 Runner)
     * @param monitor 依赖的系统监控器 (引用，生命周期需长于 Runner)
     */
    BenchmarkRunner(InferenceEngine& engine, SystemMonitor& monitor);
    ~BenchmarkRunner() = default;

    /**
     * @brief 执行压测
     * 
     * 此函数是阻塞的，直到所有请求完成。
     * 
     * @param config 压测配置
     * @return BenchmarkResult 最终统计结果
     */
    BenchmarkResult Run(const BenchmarkConfig& config);

private:
    InferenceEngine& engine_;
    SystemMonitor& monitor_;

    /**
     * @brief 计算延迟的百分位数
     * 
     * @param latencies 所有样本数据
     * @param percentile 百分位 (e.g. 0.99)
     * @return double 对应的延迟值
     */
    double CalculatePercentile(std::vector<double>& latencies, double percentile);
};
