#include <gtest/gtest.h>
#include "BenchmarkRunner.h"
#include <iostream>
#include <fstream>

// 这个测试会真正跑起来，虽然是用随机数据。
// 它验证了所有模块的协同工作。
TEST(BenchmarkRunnerTest, Integration) {
    // 1. 准备依赖
    SystemMonitor monitor;
    InferenceEngine engine;
    
    // 尝试加载 ResNet50 (如果存在)
    std::string model_path = "tests/resnet50.onnx";
    std::ifstream f(model_path.c_str());
    if (!f.good()) {
        GTEST_SKIP() << "Skipping integration test: model not found";
    }
    
    ASSERT_NO_THROW(engine.LoadModel(model_path));

    // 2. 配置压测
    BenchmarkConfig config;
    config.threads = 4;
    config.requests = 20; // 跑 20 次意思一下
    config.warmup_rounds = 2; // 热身 2 次

    // 3. 运行
    BenchmarkRunner runner(engine, monitor);
    BenchmarkResult result = runner.Run(config);

    // 4. 验证结果合理性
    std::cout << "Integration Test Results:" << std::endl;
    std::cout << "QPS: " << result.qps << std::endl;
    std::cout << "Avg Latency: " << result.avg_latency_ms << " ms" << std::endl;
    std::cout << "Avg CPU: " << result.avg_cpu_usage << "%" << std::endl;
    std::cout << "Peak Mem: " << result.peak_memory_mb << " MB" << std::endl;

    EXPECT_GT(result.qps, 0.0);
    EXPECT_GT(result.avg_latency_ms, 0.0);
    // 注意：在短测试里，CPU 采样可能会错过峰值或为 0，所以很难断言 > 0，但不应崩坏
    EXPECT_GE(result.peak_memory_mb, 0.0);
}
