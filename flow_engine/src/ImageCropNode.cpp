#include "ImageCropNode.h"

namespace flow {

    ImageCropNode::ImageCropNode(int id, const std::string& name, int width, int height)
        : Node(id, name) {
        if (width > 0) {
            setParam("width", width);
        }
        if (height > 0) {
            setParam("height", height);
        }
        setParam("x", -1);
        setParam("y", -1);

        addInputPort<cv::Mat>(
            0,
            "image",
            true,
            "输入图像（BGR/Gray，按上游约定）"
        );
        addOutputPort<cv::Mat>(
            0,
            "image",
            true,
            "裁剪后的图像（与输入通道/类型保持一致）"
        );
    }

    std::vector<PortInfo> ImageCropNode::getInputPorts() const {
        return input_ports_;
    }

    std::vector<PortInfo> ImageCropNode::getOutputPorts() const {
        return output_ports_;
    }

    ExecuteResult ImageCropNode::execute(const std::map<std::string, std::any>& inputs) {
        auto it = inputs.find("image");
        if (it == inputs.end()) {
            return ExecuteResult::Error("ImageCropNode: missing input 'image'");
        }

        cv::Mat src;
        try {
            src = std::any_cast<cv::Mat>(it->second);
        } catch (...) {
            return ExecuteResult::Error("ImageCropNode: input 'image' type is not cv::Mat");
        }

        if (src.empty()) {
            return ExecuteResult::Error("ImageCropNode: input image is empty");
        }

        const int cropW = getParam<int>("width", 0);
        const int cropH = getParam<int>("height", 0);
        if (cropW <= 0 || cropH <= 0) {
            return ExecuteResult::Error("ImageCropNode: params 'width'/'height' must be > 0");
        }

        if (cropW > src.cols || cropH > src.rows) {
            return ExecuteResult::Error("ImageCropNode: crop size is larger than input image");
        }

        int x = getParam<int>("x", -1);
        int y = getParam<int>("y", -1);

        if (x < 0) x = (src.cols - cropW) / 2;
        if (y < 0) y = (src.rows - cropH) / 2;

        if (x < 0 || y < 0 || x + cropW > src.cols || y + cropH > src.rows) {
            return ExecuteResult::Error("ImageCropNode: crop rect is out of range");
        }

        const cv::Rect roi(x, y, cropW, cropH);
        cv::Mat cropped = src(roi).clone();

        return ExecuteResult::Success({ {"image", cropped} });
    }

} // namespace flow

