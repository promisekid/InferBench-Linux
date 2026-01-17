# 项目进度与路线图: InferBench-Linux

## 第一阶段: MVP 开发 (第一周)
**目标**: 实现核心功能并完成基础 CLI 开发。

### 📅 里程碑分解
- [x] **M1: 项目搭建 (Project Setup)**
    - [x] 建立 CMake 项目结构
    - [x] Git 初始化与忽略文件配置
    - [x] 依赖配置 (引入 ONNX Runtime, GTest)
- [x] **M2: 系统监控模块 (System Monitor)**
    - [x] 实现 `SystemMonitor` 类
    - [x] 实现 `/proc/stat` 解析 (CPU)
    - [x] 实现 `/proc/self/stat` 解析 (Memory)
- [x] **M3: 推理引擎 (Inference Engine)**
    - [x] 封装 `Ort::Session`
    - [x] 实现 `LoadModel` 接口
    - [x] 实现随机 Tensor 生成器
    - [x] 实现 `Run` 推理接口
- [x] **M4: 压测核心 (Benchmark Core)**
    - [x] 实现线程池逻辑
    - [x] 实现 `BenchmarkRunner` 调度器
    - [x] 实现高精度计时与延迟统计
- [x] **M5: CLI 与集成 (Integration)**
    - [x] 命令行参数解析
    - [x] 模块组装与联调
    - [x] 终端进度条实现
    - [x] 报告输出逻辑
- [ ] **M6: MVP 验证 (Verification)**
    - [ ] ResNet50/MobileNet 跑通测试
    - [ ] 内存泄漏检查 (Valgrind)

## 第二阶段: 优化与健壮性 (第二周)
**目标**: 提升稳定性，支持更多场景，增强易用性。

- [ ] **报告增强**: 支持详细的直方图统计，ASCII 格式的延迟分布图。
- [ ] **真实数据支持**: 支持加载外部数据文件 (.bin/.npy) 而非仅随机数据。
- [ ] **性能分析**: 集成简单的 Profiling 功能 (区分 前处理/推理/后处理 耗时)。

## 第三阶段: 高级特性 (未来规划)
- [ ] **GPU 支持**: 启用 CUDA/TensorRT Execution Providers。
- [ ] **服务端模式**: 暴露 HTTP/gRPC 接口，支持远程触发压测。
