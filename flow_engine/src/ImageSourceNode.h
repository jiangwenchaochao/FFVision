#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>
#include <string>

namespace flow {

    // ImageSourceNode: 从文件路径加载图片，输出 cv::Mat
    //
    // 输出端口:
    //   - image: cv::Mat (解码后的图像)
    //
    // 参数:
    //   - path: std::string (图片文件路径，必填)
    class ImageSourceNode : public Node {
    public:
        ImageSourceNode(int id, const std::string& name, const std::string& path = {});

        std::vector<PortInfo> getOutputPorts() const override;

        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override;
    };

} // namespace flow
