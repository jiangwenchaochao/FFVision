#pragma once
#include "node_type.h"

namespace flow{


    class result_base{
        virtual ~result_base(){}
    };

    template<node_type t>
    class result:public result_base{
    };

}