#pragma once
#include "../node.h"

namespace flow{


    class result_image_local : public result<node_type::image_local> {


    };

    class image_local: node<node_type::image_local>{
    public:

        void init() override final{

            
        }
    private:
        std::string image_folder = "D://fake_image";
        int image_index = 0;
        int img_count = 0;
        float fps = 1.0;

    };


}