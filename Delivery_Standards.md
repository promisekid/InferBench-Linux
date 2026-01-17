# 交付标准: InferBench-Linux

## 1. 代码质量指标 (Code Quality)
> 详见单独文档: [Code_Quality_Standards.md](./Code_Quality_Standards.md)

## 2. 功能验收标准 (Acceptance Criteria)
1.  **加载测试**: 能够成功加载标准的 ONNX 模型 (如 ResNet50, YOLOv5, BERT-tiny)。
2.  **稳定性测试**: 能够连续运行至少 1 小时而不崩溃，且不发生 OOM (Out of Memory)。
3.  **精度要求 - 延迟**: 测量代码本身的耗时开销 (Overhead) 必须小于实际推理耗时的 1% (对于耗时 >5ms 的模型)。
4.  **精度要求 - 监控**: CPU/内存的读数应与标准工具 (如 `top` 或 `htop`) 相比，误差在 ±5% 以内。

## 3. 交付物清单 (Deliverables)
1.  **可执行文件**: 静态链接（或正确设置 RPATH）的二进制文件 `inferbench`。
2.  **脚本**: 用于 MVP 自动化验证的 `run_test.sh` 脚本。
3.  **测试报告**: 压测结果样例文件 (`benchmark_result.json`)。
4.  **文档**: 完整的 `README.md`，包含编译指南和使用示例。

## 4. 测试覆盖率 (Test Coverage)
- **单元测试**: `SystemMonitor` 和 `Stats` 统计逻辑的测试覆盖率需 > 80%。
- **集成测试**: 包含自动化脚本，跑通 Dummy 模型的全流程。
