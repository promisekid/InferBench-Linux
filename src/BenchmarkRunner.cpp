#include "BenchmarkRunner.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <mutex>
#include <numeric>

BenchmarkRunner::BenchmarkRunner(InferenceEngine& engine, SystemMonitor& monitor)
    : engine_(engine), monitor_(monitor) {}

BenchmarkResult BenchmarkRunner::Run(const BenchmarkConfig& config) {
    BenchmarkResult result;

    // 1. 准备测试数据 (Random Input)
    int64_t input_size = engine_.GetInputSize();
    std::vector<float> input_data(input_size);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (auto& val : input_data) val = dist(gen);

    // 2. 预热 (Warmup)
    // 目的：让 CPU caches 热起来 & 触发 ONNX Runtime 内部可能的 JIT/Allocations
    for (int i = 0; i < config.warmup_rounds; ++i) {
        engine_.Run(input_data);
    }

    // 3. 准备并发控制
    std::atomic<int> remaining_requests(config.requests);
    std::vector<std::thread> threads;
    std::mutex stats_mutex; // 仅用于最后合并数据
    
    // Per-thread statistics to avoid lock verify
    // 这种做法叫 TLS (Thread Local Storage) 思想的手动实现
    std::vector<std::vector<double>> all_thread_latencies(config.threads);

    // 4. 启动系统监控线程
    std::atomic<bool> monitor_running(true);
    std::vector<double> cpu_samples;
    std::vector<double> mem_samples;
    
    std::thread monitor_thread([&]() {
        while (monitor_running) {
            cpu_samples.push_back(monitor_.GetCpuUsage());
            mem_samples.push_back(monitor_.GetMemoryUsage());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    // 5. 启动 Worker 线程
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int t = 0; t < config.threads; ++t) {
        threads.emplace_back([&, t]() {
            while (true) {
                // 抢单：原子减
                // fetch_sub 返回修改前的值
                int current_req_idx = remaining_requests.fetch_sub(1);
                if (current_req_idx <= 0) {
                    break; // 抢没了，下班
                }

                auto t1 = std::chrono::high_resolution_clock::now();
                engine_.Run(input_data);
                auto t2 = std::chrono::high_resolution_clock::now();

                // 记录延迟 (毫秒)
                double lat_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
                all_thread_latencies[t].push_back(lat_ms);
            }
        });
    }

    // 6. 等待所有 Worker 结束
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_time_sec = std::chrono::duration<double>(end_time - start_time).count();

    // 7. 停止监控
    monitor_running = false;
    if (monitor_thread.joinable()) monitor_thread.join();

    // 8. 汇总数据
    // 合并 Latency
    std::vector<double> flat_latencies;
    flat_latencies.reserve(config.requests);
    for (const auto& local_lats : all_thread_latencies) {
        flat_latencies.insert(flat_latencies.end(), local_lats.begin(), local_lats.end());
    }

    // 计算统计指标
    result.qps = config.requests / total_time_sec;
    
    if (!flat_latencies.empty()) {
        double sum = std::accumulate(flat_latencies.begin(), flat_latencies.end(), 0.0);
        result.avg_latency_ms = sum / flat_latencies.size();
        result.p99_latency_ms = CalculatePercentile(flat_latencies, 0.99);
    }

    if (!cpu_samples.empty()) {
        double sum = std::accumulate(cpu_samples.begin(), cpu_samples.end(), 0.0);
        result.avg_cpu_usage = sum / cpu_samples.size();
    }

    if (!mem_samples.empty()) {
        // 峰值内存
        result.peak_memory_mb = *std::max_element(mem_samples.begin(), mem_samples.end());
    }

    return result;
}

double BenchmarkRunner::CalculatePercentile(std::vector<double>& latencies, double percentile) {
    if (latencies.empty()) return 0.0;
    
    // 排序以找到 P99
    std::sort(latencies.begin(), latencies.end());
    
    // index = ceil(p / 100 * N) - 1
    // 或简单的 index = p * N
    size_t index = static_cast<size_t>(percentile * latencies.size());
    
    // 边界保护
    if (index >= latencies.size()) index = latencies.size() - 1;
    
    return latencies[index];
}
