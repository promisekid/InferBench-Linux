#include "InferenceEngine.h"
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <cstring>

InferenceEngine::InferenceEngine() 
    : env_(ORT_LOGGING_LEVEL_WARNING, "InferBench") {
}

InferenceEngine::~InferenceEngine() {
    for (auto name : input_node_names_) {
        free(const_cast<char*>(name));
    }
    for (auto name : output_node_names_) {
        free(const_cast<char*>(name));
    }
}

void InferenceEngine::LoadModel(const std::string& model_path, int opt_level) {
    Ort::SessionOptions session_options;
    
    // Set Optimization Level
    switch (opt_level) {
        case 0:
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
            break;
        case 1:
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
            break;
        case 2:
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
            break;
        case 99:
        default:
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
            break;
    }

    session_options.SetIntraOpNumThreads(1);

    try {
        session_ = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options);
    } catch (const Ort::Exception& e) {
        throw std::runtime_error("Failed to load model: " + std::string(e.what()));
    }

    Ort::AllocatorWithDefaultOptions allocator;

    // 处理输入节点
    size_t num_input_nodes = session_->GetInputCount();
    
    for(size_t i = 0; i < num_input_nodes; i++) {
        auto input_name_ptr = session_->GetInputNameAllocated(i, allocator);
        // 深拷贝 name，因为 API 返回的 smart pointer 会自动释放，而我们需要在 Run 中长期持有
        input_node_names_.push_back(strdup(input_name_ptr.get()));

        Ort::TypeInfo type_info = session_->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        
        input_node_dims_ = tensor_info.GetShape();
        
        // 处理动态维度 (Batch Size = -1)，强制设为 1
        for (auto& dim : input_node_dims_) {
            if (dim < 1) dim = 1;
        }

        input_tensor_size_ = 1;
        for (auto dim : input_node_dims_) {
            input_tensor_size_ *= dim;
        }
    }

    // 处理输出节点
    size_t num_output_nodes = session_->GetOutputCount();
    for(size_t i = 0; i < num_output_nodes; i++) {
        auto output_name_ptr = session_->GetOutputNameAllocated(i, allocator);
        output_node_names_.push_back(strdup(output_name_ptr.get()));
    }
}

std::vector<float> InferenceEngine::Run(const std::vector<float>& input_data) {
    if (input_data.size() != input_tensor_size_) {
        throw std::runtime_error("Input data size mismatch!");
    }

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    // Zero-Copy 创建 input tensor
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info, 
        const_cast<float*>(input_data.data()), 
        input_data.size(), 
        input_node_dims_.data(), 
        input_node_dims_.size()
    );

    auto output_tensors = session_->Run(
        Ort::RunOptions{nullptr}, 
        input_node_names_.data(), 
        &input_tensor, 
        1,
        output_node_names_.data(), 
        1
    );

    float* floatarr = output_tensors[0].GetTensorMutableData<float>();
    auto output_tensor_info = output_tensors[0].GetTensorTypeAndShapeInfo();
    size_t output_size = output_tensor_info.GetElementCount();

    return std::vector<float>(floatarr, floatarr + output_size);
}

int64_t InferenceEngine::GetInputSize() const {
    return static_cast<int64_t>(input_tensor_size_);
}
