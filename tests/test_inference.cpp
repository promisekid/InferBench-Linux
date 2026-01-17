#include <gtest/gtest.h>
#include "InferenceEngine.h"
#include <numeric>
#include <vector>
#include <random>
#include <fstream>

TEST(InferenceEngineTest, LoadModelThrowsOnMissingFile) {
    InferenceEngine engine;
    EXPECT_THROW(engine.LoadModel("non_existent_model.onnx"), std::runtime_error);
}

TEST(InferenceEngineTest, CanRunResNet50) {
    InferenceEngine engine;
    
    // 尝试加载刚刚下载的模型
    // 注意：如果网络问题导致下载失败，这个测试会 fail。
    // 在真实 CI 中应该 mock 或者保证文件存在。
    std::string model_path = "tests/resnet50.onnx";
    std::ifstream f(model_path.c_str());
    if (!f.good()) {
        GTEST_SKIP() << "Skipping test: resnet50.onnx not found";
    }

    ASSERT_NO_THROW(engine.LoadModel(model_path));

    // 获取输入大小 (ResNet50 -> 1x3x224x224 = 150528)
    int64_t input_size = engine.GetInputSize();
    EXPECT_GT(input_size, 0);

    // 生成随机数据
    std::vector<float> input_data(input_size);
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for(auto& val : input_data) val = dist(gen);

    // 执行推理
    std::vector<float> output;
    ASSERT_NO_THROW(output = engine.Run(input_data));

    // 验证输出 (ResNet50 ImageNet 1000 classes)
    EXPECT_EQ(output.size(), 1000);
    
    // 简单的数值检查
    float sum = std::accumulate(output.begin(), output.end(), 0.0f);
    EXPECT_FALSE(std::isnan(sum));
}
