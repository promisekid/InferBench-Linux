# 指定 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称、版本号以及使用的编程语言（C++）
project(InferBench-Linux VERSION 0.1.0 LANGUAGES CXX)

# 设置 C++ 标准
# 指定使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
# 强制要求编译器支持所选的 C++ 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译产物的输出目录
# 设置静态库（.a）的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置动态库（.so）的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 设置可执行二进制文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 指定头文件搜索路径
include_directories(include)
include_directories(third_party/onnxruntime/include)

# 指定库文件搜索路径
link_directories(third_party/onnxruntime/lib)

# --- GoogleTest Integration ---
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# -----------------------------

# 查找所有源文件 
# file(GLOB...) 在工程变大时不是最佳实践，但 MVP 阶段为了方便先保留，或者手动列出
set(SOURCES
    src/test_ort.cpp
    src/SystemMonitor.cpp
    src/InferenceEngine.cpp
    src/BenchmarkRunner.cpp
)

# 添加可执行文件 (Main App)
add_executable(inferbench 
    src/main.cpp
    src/SystemMonitor.cpp
    src/InferenceEngine.cpp
    src/BenchmarkRunner.cpp
)
target_link_libraries(inferbench onnxruntime)

# --- Unit Tests ---
enable_testing()
add_executable(unit_tests 
    tests/test_main.cpp
    tests/test_monitor.cpp
    tests/test_inference.cpp
    tests/test_benchmark.cpp
    src/SystemMonitor.cpp
    src/InferenceEngine.cpp
    src/BenchmarkRunner.cpp
)
target_link_libraries(unit_tests GTest::gtest_main onnxruntime)

include(GoogleTest)
gtest_discover_tests(unit_tests)
