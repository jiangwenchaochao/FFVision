#pragma once
#include "node_type.h"

namespace flow{

    struct result_base{
    public:
        virtual ~result_base(){}
    };

    template<node_type t>
    struct result:public result_base{};

}