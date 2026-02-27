#include "ImageSaveNode.h"

#include <exception>
#include <filesystem>

namespace flow {

    ImageSaveNode::ImageSaveNode(int id, const std::string& name,
                                 const std::string& directory,
                                 const std::string& filename)
        : Node(id, name) {
        if (!directory.empty()) {
            setParam("directory", directory);
        }
        setParam("filename", filename.empty() ? std::string("image.png") : filename);
        setParam("ext", std::string(".png"));
        addInputPort<cv::Mat>(
            0,
            "image",
            true,
            "待保存的图像"
        );
        addOutputPort<std::string>(
            0,
            "path",
            true,
            "最终写入磁盘的完整路径"
        );
    }

    std::vector<PortInfo> ImageSaveNode::getInputPorts() const {
        return input_ports_;
    }

    std::vector<PortInfo> ImageSaveNode::getOutputPorts() const {
        return output_ports_;
    }

    static std::filesystem::path ensureExtension(std::filesystem::path p, const std::string& ext) {
        if (p.has_extension()) {
            return p;
        }
        if (ext.empty()) {
            return p;
        }
        std::string e = ext;
        if (!e.empty() && e.front() != '.') {
            e.insert(e.begin(), '.');
        }
        return p.replace_extension(e);
    }

    ExecuteResult ImageSaveNode::execute(const std::map<std::string, std::any>& inputs) {
        auto it = inputs.find("image");
        if (it == inputs.end()) {
            return ExecuteResult::Error("ImageSaveNode: missing input 'image'");
        }

        cv::Mat src;
        try {
            src = std::any_cast<cv::Mat>(it->second);
        } catch (...) {
            return ExecuteResult::Error("ImageSaveNode: input 'image' type is not cv::Mat");
        }

        if (src.empty()) {
            return ExecuteResult::Error("ImageSaveNode: input image is empty");
        }

        const std::string explicitPath = getParam<std::string>("path", "");
        const std::string directory = getParam<std::string>("directory", "");
        const std::string filename = getParam<std::string>("filename", "");
        const std::string ext = getParam<std::string>("ext", ".png");

        std::filesystem::path outPath;
        if (!explicitPath.empty()) {
            outPath = std::filesystem::path(explicitPath);
        } else {
            if (filename.empty()) {
                return ExecuteResult::Error("ImageSaveNode: param 'filename' is empty");
            }
            outPath = directory.empty()
                ? std::filesystem::path(filename)
                : (std::filesystem::path(directory) / filename);
            outPath = ensureExtension(outPath, ext);
        }

        std::error_code ec;
        const auto parent = outPath.parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                return ExecuteResult::Error("ImageSaveNode: failed to create directory: " + parent.string());
            }
        }

        bool ok = false;
        try {
            ok = cv::imwrite(outPath.string(), src);
        } catch (const std::exception& e) {
            return ExecuteResult::Error(std::string("ImageSaveNode: imwrite exception: ") + e.what());
        } catch (...) {
            return ExecuteResult::Error("ImageSaveNode: imwrite unknown exception");
        }

        if (!ok) {
            return ExecuteResult::Error("ImageSaveNode: failed to save image: " + outPath.string());
        }

        return ExecuteResult::Success({ {"path", outPath.string()} });
    }

} // namespace flow

