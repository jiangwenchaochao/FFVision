#pragma once
#include "../node.h"
#include <iostream>
#include <filesystem>
#include <vector>

namespace cv {
    class Mat;
}

namespace flow{

    class image{
    public:
        image(cv::Mat & mat, int id) noexcept{
            this->mat_ = mat;
            this->frame_id_ = id;
        }

        image(const image& img) noexcept{
            this->mat_ = img.mat_;
            this->frame_id_ = img.frame_id_;
        }

        image(image&& img) noexcept{
            this->mat_ = std::move(img.mat_);
            frame_id_ =img.frame_id_;
        }

        image & operator = (const image & img) noexcept{
            this->mat_ = img.mat_;
            this->frame_id_ = img.frame_id_;
            return *this;
        }

    public:
        cv::Mat mat_;
        int frame_id_ = 0;
    };

    template<>
    struct result<node_type::image_local>{
        image image_;
    };

    class image_local: public node<node_type::image_local>{
    public:
        
        void init() override final;

        void stop() override final;

        void exec(context & context) override final;

    private:
        std::string image_folder = "D://fake_image";
        int img_index_ = 0;
        int img_count_ = 0;
        bool is_folder = false;
        float fps = 1.0;
        std::vector<std::filesystem::path> image_paths_;
    };


}