# 项目进度与路线图: InferBench-Linux

## 第一阶段: MVP 开发 (已完成)
**目标**: 实现核心功能并完成基础 CLI 开发。

### 📅 里程碑分解
- [x] **M1: 项目搭建 (Project Setup)**
- [x] **M2: 系统监控模块 (System Monitor)**
- [x] **M3: 推理引擎 (Inference Engine)**
- [x] **M4: 压测核心 (Benchmark Core)**
- [x] **M5: CLI 与集成 (Integration)**

---

## 第二阶段: v0.2.0 工具链升级 (36小时冲刺)
**目标**: 升级为嵌入式 AI 工具链组件，支持资源保护、模型探查与自动化流水线。

### 📅 里程碑分解 (New)
- [ ] **M6: 资源保护 (Resource Watchdog)**
    - [ ] 实现 `SystemMonitor` 内存监控 (RSS)
    - [ ] 实现 `--memory_limit` 熔断机制
- [ ] **M7: 模型探查与优化 (Model Probe & Optimization)**
    - [ ] 实现 `--probe` 元数据打印
    - [ ] 集成 `SessionOptions` 图优化级别配置
- [ ] **M8: 自动化套件 (Automation Suite)**
    - [ ] 开发 `scripts/benchmark_suite.py` (多线程压测 + 绘图)
    - [ ] 开发 `scripts/mem_check.sh` (ASan 集成)
- [ ] **M9: CI/CD 集成**
    - [ ] 配置 GitHub Actions (`.github/workflows/ci.yml`)

---

## 第三阶段: 高级特性 (未来规划)
- [ ] **GPU 支持**: 启用 CUDA/TensorRT Execution Providers。
- [ ] **服务端模式**: 暴露 HTTP/gRPC 接口，支持远程触发压测。
