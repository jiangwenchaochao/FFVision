#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>
#include <string>

namespace flow {

    // ImageCropNode: 裁剪输入图像，输出裁剪后的 cv::Mat
    //
    // 输入端口:
    //   - image: cv::Mat
    //
    // 输出端口:
    //   - image: cv::Mat
    //
    // 参数:
    //   - width : int (必填，裁剪宽度 > 0)
    //   - height: int (必填，裁剪高度 > 0)
    //   - x     : int (可选，左上角 X；<0 表示居中)
    //   - y     : int (可选，左上角 Y；<0 表示居中)
    class ImageCropNode : public Node {
    public:
        ImageCropNode(int id, const std::string& name, int width = 0, int height = 0);

        std::vector<PortInfo> getInputPorts() const override;
        std::vector<PortInfo> getOutputPorts() const override;

        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override;
    };

} // namespace flow

