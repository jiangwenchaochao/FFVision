#include "ImageSourceNode.h"

namespace flow {

    ImageSourceNode::ImageSourceNode(int id, const std::string& name, const std::string& path)
        : Node(id, name)
    {
        if (!path.empty()) {
            setParam("path", path);
        }

        addOutputPort<cv::Mat>(
            0,
            "image",
            true,
            "从文件解码得到的图像（BGR，8-bit）"
        );
    }

    std::vector<PortInfo> ImageSourceNode::getOutputPorts() const {
        return output_ports_;
    }

    ExecuteResult ImageSourceNode::execute(const std::map<std::string, std::any>& /*inputs*/) {
        const std::string path = getParam<std::string>("path", "");
        if (path.empty()) {
            return ExecuteResult::Error("ImageSourceNode: param 'path' is empty");
        }

        cv::Mat image = cv::imread(path, cv::IMREAD_COLOR);
        if (image.empty()) {
            return ExecuteResult::Error("ImageSourceNode: failed to load image: " + path);
        }

        return ExecuteResult::Success({{"image", image}});
    }

} // namespace flow
