# 实施计划 - InferBench-Linux v0.2.0

## 目标描述
将 InferBench-Linux 升级到 v0.2.0，使其作为嵌入式 AI 工具链的一个综合组件。这包括添加资源保护 (看门狗)、模型内省 (探查)、自动化压测脚本和 CI/CD 集成。

## 建议变更

### 核心引擎 (C++)

#### [修改] [include/InferenceEngine.h](file:///home/pro/InferBench-Linux/include/InferenceEngine.h)
- 修改 `LoadModel` 签名：`void LoadModel(const std::string& path, int opt_level = 99);`
- `opt_level` 映射: 0=Basic, 1=Extended, 99=All (Default)。


#### [修改] [main.cpp](file:///home/pro/InferBench-Linux/src/main.cpp)
- 添加命令行参数解析：`--memory_limit`, `--optimization`, 和 `--probe`。
- 在推理循环中集成 `SystemMonitor::CheckMemoryLimit` 检查。
- 实现处理 `--probe` 模式的逻辑（检查模型并退出）。
- 根据 `--optimization` 标志配置 `Ort::SessionOptions` 并传给 `LoadModel`。

#### [修改] [src/SystemMonitor.cpp](file:///home/pro/InferBench-Linux/src/SystemMonitor.cpp)
- 在 `SystemMonitor` 类中添加 `bool CheckMemoryLimit(double limit_mb)` 方法。
- 复用现有的 `/proc/self/statm` 读取逻辑。

### 自动化层 (Python/Shell)

#### [新增] [scripts/benchmark_suite.py](file:///home/pro/InferBench-Linux/scripts/benchmark_suite.py)
- 脚本：使用 1, 2, 4, 8, 16 线程运行 `inferbench`。
- 解析输出并使用 Matplotlib 生成 `latency.png` 和 `throughput.png`。

#### [新增] [scripts/mem_check.sh](file:///home/pro/InferBench-Linux/scripts/mem_check.sh)
- 脚本：使用 `-fsanitize=address` 编译并运行测试/基准测试以检查泄漏。

### 构建系统 & CI

#### [新增] [cmake/arm-linux.toolchain.cmake](file:///home/pro/InferBench-Linux/cmake/arm-linux.toolchain.cmake)
- 创建一个标准的 CMake 交叉编译配置文件模板。

#### [修改] [CMakeLists.txt](file:///home/pro/InferBench-Linux/CMakeLists.txt)
- 添加 AddressSanitizer (ASan) 选项。
- 确保 Python 脚本被复制或可访问。

#### [新增] [.github/workflows/ci.yml](file:///home/pro/InferBench-Linux/.github/workflows/ci.yml)
- GitHub Actions 配置：在 push/pull_request 时构建和测试。

## 验证计划

### 自动化测试
- 运行 `scripts/benchmark_suite.py` 确保其完成并生成图像。
- 运行 `scripts/mem_check.sh` 确保 ASan 运行无误。
- 参数解析和看门狗逻辑的单元测试（如果即使拆分）。

### 手动验证
- 使用 `--probe` 对示例模型运行，验证元数据输出。
- 设置较低的 `--memory_limit` 运行，验证是否优雅退出。
