#pragma once
#include "context.h"
#include "result.h"
#include "node_type.h"
#include "result.h"

namespace flow{

class node_base{

public:
    ~node_base(){};

    virtual void init() = 0;
    virtual void exec(context & context) = 0;
    virtual long long get_id() = 0;
    virtual void set_id(long long id) = 0;
    virtual result_base * last_result() = 0; //最后一次运行的运行数据
};

template<node_type Type>
class node:public node_base{

public:
    constexpr static node_type TYPE = Type;
    static node_type get_type(){return TYPE;}

public:
    void set_id(long long id) override final{
        this->id_ = id;
    }
    long long get_id(long long id) const override final{
        return this->id_;
    }
    result<Type>& last_result() override final{
        return result_;
    }
private:
    long long id_ = 0;

    result<Type> result_;
};
}
