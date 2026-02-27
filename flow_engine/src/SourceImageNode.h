#pragma once

#include "Node.h"
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace flow {

    // SourceImageNode: 读取图片文件的原始字节并输出
    //
    // 输出:
    // - image: std::vector<uint8_t> (文件原始字节)
    // - path : std::string (图片路径)
    //
    // 参数:
    // - path: std::string (必填)
    class SourceImageNode : public Node {
    public:
        SourceImageNode(int id, const std::string& name, const std::string& path = {})
            : Node(id, name) {
            if (!path.empty()) {
                setParam("path", path);
            }

            addOutputPort<std::vector<std::uint8_t>>(
                0,
                "image",
                true,
                "图片文件原始字节（未解码）"
            );
            addOutputPort<std::string>(
                1,
                "path",
                false,
                "输入图片路径"
            );
        }

        std::vector<PortInfo> getOutputPorts() const override {
            return output_ports_;
        }

        ExecuteResult execute(const std::map<std::string, std::any>& /*inputs*/) override {
            const std::string path = getParam<std::string>("path", "");
            if (path.empty()) {
                return ExecuteResult::Error("SourceImageNode: param 'path' is empty");
            }

            std::ifstream ifs(path, std::ios::binary);
            if (!ifs) {
                return ExecuteResult::Error("SourceImageNode: cannot open file: " + path);
            }

            std::vector<std::uint8_t> bytes(
                std::istreambuf_iterator<char>(ifs),
                std::istreambuf_iterator<char>()
            );

            std::map<std::string, std::any> out;
            out["image"] = bytes;
            out["path"] = path;
            return ExecuteResult::Success(out);
        }
    };

} // namespace flow

