#!/bin/bash
set -e

echo "=== Memory Safety Check (ASan) ==="

# 1. Clean and Rebuild with ASan
echo "[Build] Compiling with AddressSanitizer..."
rm -rf build_asan
mkdir -p build_asan
cd build_asan
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON ..
make -j$(nproc)

# 2. Run Unit Tests
echo "[Test] Running Unit Tests..."
./bin/unit_tests

# 3. Run Benchmark (Integration Test)
# Use a small number of requests just to check for leaks during execution
echo "[Test] Running InferenceEngine Integration..."
# Assuming we have the test model available
MODEL_PATH="../tests/resnet50.onnx"
if [ ! -f "$MODEL_PATH" ]; then
    echo "Warning: Test model not found at $MODEL_PATH, skipping integration run."
else
    # Run with leak check enabled (ASan default)
    # We add export ASAN_OPTIONS=detect_leaks=1 just to be explicit, though usually default on Linux x86_64
    export ASAN_OPTIONS=detect_leaks=1:symbolize=1
    ./bin/inferbench --model "$MODEL_PATH" --requests 10 --threads 2 --optimization basic
fi

echo "=== Memory Check Passed! ==="
