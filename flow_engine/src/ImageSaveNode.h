#pragma once

#include "Node.h"
#include <opencv2/opencv.hpp>
#include <string>

namespace flow {

    // ImageSaveNode: 将输入 cv::Mat 保存到磁盘
    //
    // 输入端口:
    //   - image: cv::Mat
    //
    // 输出端口:
    //   - path : std::string (最终保存路径)
    //
    // 参数:
    //   - path     : std::string (可选，完整保存路径；若设置则优先生效)
    //   - directory: std::string (可选，保存目录；为空则使用当前工作目录)
    //   - filename : std::string (必填，文件名；可含扩展名)
    //   - ext      : std::string (可选，默认 ".png"；当 filename 无扩展名时生效)
    class ImageSaveNode : public Node {
    public:
        ImageSaveNode(int id, const std::string& name,
                      const std::string& directory = {},
                      const std::string& filename = "image.png");

        std::vector<PortInfo> getInputPorts() const override;
        std::vector<PortInfo> getOutputPorts() const override;

        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override;
    };

} // namespace flow

