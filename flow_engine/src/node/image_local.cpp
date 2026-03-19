#include "image_local.h"

#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <algorithm>

namespace flow{
    void image_local::stop() {
        
    }
    void image_local::init() {
        namespace fs = std::filesystem;

        fs::path folder(image_folder);
        if (!fs::exists(folder) || !fs::is_directory(folder)) {
            throw std::runtime_error("image_local: image_folder is not a valid directory: " + image_folder);
        }

        image_paths_.clear();

        for (const auto & entry : fs::directory_iterator(folder)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto & p = entry.path();
            const auto ext = p.extension().string();

            // 只接受常见图片扩展名
            if (ext == ".jpg" || ext == ".png" || ext == ".bmp" ||
                ext == ".jpeg" || ext == ".JPG" || ext == ".PNG" ||
                ext == ".BMP" || ext == ".JPEG") {
                image_paths_.push_back(p);
            }
        }

        // 可选：按文件名排序，保证顺序稳定
        std::sort(image_paths_.begin(), image_paths_.end());
        img_count_ = static_cast<int>(image_paths_.size());
        img_index_ = 0;
        is_folder = true;

        if (img_count_ == 0) {
            throw std::runtime_error("image_local: no images found in folder: " + image_folder);
        }
    }

    void image_local::exec(context & context) {
        using namespace std::chrono;
        if (img_count_ <= 0 || image_paths_.empty()) {
            break;
        }
        const auto & path = image_paths_[img_index_];
        cv::Mat frame = cv::imread(path.string(), cv::IMREAD_COLOR);
        if (frame.empty()) {
            img_index_ = (img_index_ + 1) % img_count_;
            std::this_thread::sleep_for(frame_interval);
            continue;
        }
        image img = new image(frame,img_index_);
        result_.image_ = img;
        img_index_ = (img_index_ + 1) % img_count_;
        context.set("image", img);
    }
}