#!/bin/bash
set -e

# 定义版本和下载链接
ORT_VERSION="1.16.3"
ORT_FILE="onnxruntime-linux-x64-${ORT_VERSION}.tgz"
ORT_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ORT_VERSION}/${ORT_FILE}"

#以此脚本所在目录为基准，找到项目根目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="${SCRIPT_DIR}/.."
THIRD_PARTY_DIR="${PROJECT_ROOT}/third_party"

mkdir -p "${THIRD_PARTY_DIR}"

# 检查是否已经存在
if [ -d "${THIRD_PARTY_DIR}/onnxruntime-linux-x64-${ORT_VERSION}" ]; then
    echo "ONNX Runtime ${ORT_VERSION} already exists."
    exit 0
fi

echo "Downloading ONNX Runtime ${ORT_VERSION}..."
wget -q --show-progress "${ORT_URL}" -O "${THIRD_PARTY_DIR}/${ORT_FILE}"

echo "Extracting..."
tar -xzf "${THIRD_PARTY_DIR}/${ORT_FILE}" -C "${THIRD_PARTY_DIR}"

# 清理压缩包
rm "${THIRD_PARTY_DIR}/${ORT_FILE}"

# 创建一个符号链接方便引用 (onnxruntime -> onnxruntime-linux-x64-1.16.3)
# 这样以后升级版本只需要改链接，不用改 CMakeLists.txt
ln -sfn "${THIRD_PARTY_DIR}/onnxruntime-linux-x64-${ORT_VERSION}" "${THIRD_PARTY_DIR}/onnxruntime"

echo "Success! ONNX Runtime is installed in ${THIRD_PARTY_DIR}/onnxruntime"
