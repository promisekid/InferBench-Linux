#pragma once

#include <string>
#include <vector>
#include <memory>
#include <onnxruntime_cxx_api.h>

/**
 * @brief 推理引擎类，封装 ONNX Runtime 的核心功能。
 * 
 * 负责模型的加载、Tensor 内存管理以及执行推理。
 */
class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();

    /**
     * @brief 加载 ONNX 模型。
     * 
     * @param model_path 模型文件的路径 (.onnx)。
     * @param opt_level 优化级别 (0=Disable, 1=Basic, 99=All). Default: 99.
     * @throws std::runtime_error 如果加载失败。
     */
    void LoadModel(const std::string& model_path, int opt_level = 99);

    /**
     * @brief 执行推理。
     * 
     * @param input_data 输入数据（展平的 float 数组）。
     * @return std::vector<float> 推理结果（展平的 float 数组）。
     */
    std::vector<float> Run(const std::vector<float>& input_data);

    /**
     * @brief 获取模型需要的输入 Tensor 元素总数。
     * 
     * 用于生成符合大小的随机测试数据。
     * 
     * @return int64_t 元素个数 (如 1x3x224x224 = 150528)。
     */
    int64_t GetInputSize() const;

private:
    // ONNX Runtime 环境，整个进程通常只需要一个
    Ort::Env env_;
    // 会话对象，非线程安全（但在 Run 时只读模型是安全的，如果包含状态则需注意）
    // 为了更安全的并发，通常 session 是线程安全的，但 session options 不是。
    std::unique_ptr<Ort::Session> session_;
    
    // 模型的元数据 (Metadata)
    std::vector<const char*> input_node_names_;
    std::vector<const char*> output_node_names_;
    std::vector<int64_t> input_node_dims_;
    size_t input_tensor_size_ = 0;
};
