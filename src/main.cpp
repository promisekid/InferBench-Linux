#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <getopt.h>
#include <iomanip>

#include "InferenceEngine.h"
#include "SystemMonitor.h"
#include "BenchmarkRunner.h"

// 打印模型元数据
void PrintModelInfo(InferenceEngine& engine) {
    std::cout << "[Probe] Model Inspector:\n";
    // 打印输入 Tensor 的基本信息
    std::cout << "  Input Size (elements): " << engine.GetInputSize() << "\n";
    std::cout << "  (More details would require InferenceEngine API expansion)\n";
}

// 打印使用说明
void PrintUsage(const char* name) {
    std::cout << "Usage: " << name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -m, --model <path>      Path to ONNX model file (Required)\n"
              << "  -t, --threads <num>     Number of threads (Default: 1)\n"
              << "  -n, --requests <num>    Total number of requests (Default: 100)\n"
              << "  -w, --warmup <num>      Warmup rounds (Default: 10)\n"
              << "  -l, --memory_limit <MB> Memory Limit in MB (Default: 0, no limit)\n"
              << "  -o, --optimization <lvl> Optimization level: basic, all, none (Default: all)\n"
              << "  --probe                 Print model metadata and exit\n"
              << "  -j, --json <path>       Save report to JSON file\n"
              << "  -h, --help              Show this help message\n";
}

int main(int argc, char** argv) {
    // 默认配置
    std::string model_path;
    std::string json_path;
    std::string opt_str = "all";
    bool probe_mode = false;
    BenchmarkConfig config;

    // 解析命令行参数
    struct option long_options[] = {
        {"model", required_argument, 0, 'm'},
        {"threads", required_argument, 0, 't'},
        {"requests", required_argument, 0, 'n'},
        {"warmup", required_argument, 0, 'w'},
        {"memory_limit", required_argument, 0, 'l'},
        {"optimization", required_argument, 0, 'o'},
        {"probe", no_argument, 0, 'p'},
        {"json", required_argument, 0, 'j'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "m:t:n:w:l:o:p:j:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm': model_path = optarg; break;
            case 't': config.threads = std::stoi(optarg); break;
            case 'n': config.requests = std::stoi(optarg); break;
            case 'w': config.warmup_rounds = std::stoi(optarg); break;
            case 'l': config.memory_limit_mb = std::stod(optarg); break;
            case 'o': opt_str = optarg; break;
            case 'p': probe_mode = true; break;
            case 'j': json_path = optarg; break;
            case 'h': PrintUsage(argv[0]); return 0;
            default: PrintUsage(argv[0]); return 1;
        }
    }

    // 校验必填参数
    if (model_path.empty()) {
        std::cerr << "Error: --model argument is required.\n";
        PrintUsage(argv[0]);
        return 1;
    }

    std::cout << "========================================" << std::endl;
    std::cout << " InferBench-Linux v0.1.0 (MVP) " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Model: " << model_path << std::endl;
    std::cout << "Threads: " << config.threads << std::endl;
    std::cout << "Requests: " << config.requests << std::endl;
    std::cout << "Warmup: " << config.warmup_rounds << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    try {
        // 1. 初始化模块
        std::cout << "[Init] Initializing Modules..." << std::endl;
        SystemMonitor monitor;
        InferenceEngine engine;

        // 2. 加载模型
        std::cout << "[Init] Loading Model..." << std::endl;
        
        int opt_level = 99; // Default all
        if (opt_str == "basic") opt_level = 1;
        else if (opt_str == "none") opt_level = 0;
        else if (opt_str == "all") opt_level = 99;
        else {
            std::cerr << "Warning: Unknown optimization level '" << opt_str << "', using 'all'." << std::endl;
        }

        engine.LoadModel(model_path, opt_level);

        // 如果是 Probe 模式，打印信息后退出
        if (probe_mode) {
           PrintModelInfo(engine);
           return 0;
        }

        // 3. 执行压测
        std::cout << "[Run] Starting Benchmark..." << std::endl;
        BenchmarkRunner runner(engine, monitor);
        BenchmarkResult result = runner.Run(config);

        // 4. 输出报告
        std::cout << "----------------------------------------" << std::endl;
        std::cout << " Benchmark Results " << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "QPS:            " << result.qps << std::endl;
        std::cout << "Avg Latency:    " << result.avg_latency_ms << " ms" << std::endl;
        std::cout << "P99 Latency:    " << result.p99_latency_ms << " ms" << std::endl;
        std::cout << "Avg CPU Usage:  " << result.avg_cpu_usage << " %" << std::endl;
        std::cout << "Peak Memory:    " << result.peak_memory_mb << " MB" << std::endl;
        std::cout << "========================================" << std::endl;

        // 5. 保存 JSON
        if (!json_path.empty()) {
            std::ofstream json_file(json_path);
            if (json_file.is_open()) {
                json_file << "{\n";
                json_file << "  \"model\": \"" << model_path << "\",\n";
                json_file << "  \"config\": {\n";
                json_file << "    \"threads\": " << config.threads << ",\n";
                json_file << "    \"requests\": " << config.requests << "\n";
                json_file << "  },\n";
                json_file << "  \"result\": {\n";
                json_file << "    \"qps\": " << result.qps << ",\n";
                json_file << "    \"avg_latency_ms\": " << result.avg_latency_ms << ",\n";
                json_file << "    \"p99_latency_ms\": " << result.p99_latency_ms << ",\n";
                json_file << "    \"avg_cpu_usage\": " << result.avg_cpu_usage << ",\n";
                json_file << "    \"peak_memory_mb\": " << result.peak_memory_mb << "\n";
                json_file << "  }\n";
                json_file << "}\n";
                std::cout << "[Report] Saved to " << json_path << std::endl;
            } else {
                std::cerr << "[Error] Failed to save JSON report." << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "\n[Fatal Error] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
