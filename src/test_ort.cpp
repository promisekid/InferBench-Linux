#include <iostream>
#include <onnxruntime_cxx_api.h>

int main() {
    std::cout << "ONNX Runtime Version: " << Ort::GetVersionString() << std::endl;
    return 0;
}
