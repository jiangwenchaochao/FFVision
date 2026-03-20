#pragma once

/**
copy right 
jiang 3/20 2026
*/
#include "context.h"
#include "result.h"
#include "node_type.h"
#include "result.h"
#include <typeindex>
#include <vector>

namespace flow{

// 端口元信息
struct portinfo {
    int  index;
    std::string name;
    std::type_index type;
};
// 类型擦除的输入端口
struct InputPortBase {
    virtual ~InputPortBase() = default;
    virtual void setAny(const std::any& value) = 0;
    virtual const std::any& getAny() const = 0;
};

// 带类型的输入端口
template<typename T>
struct InputPort : InputPortBase {
    T value{};
    std::any storage;
    void set(const T& v) { value = v; storage = v; }
    const T& get() const { return value; }

    void setAny(const std::any& v) override {
        value = std::any_cast<T>(v);
        storage = v;
    }
    const std::any& getAny() const override { return storage; }
};


class node_base{
public:
    ~node_base(){};
    virtual void init() = 0;
    virtual void stop() = 0;
    virtual void exec(context & context) = 0;
    virtual long long get_id() = 0;
    virtual void set_id(long long id) = 0;
    virtual result_base * last_result() = 0; //最后一次运行的运行数据
    virtual std::vector<InputPortBase *> get_input_port() = 0; 
    virtual std::vector<InputPortBase *> get_output_port() = 0; 
};

template<node_type T>
class node:public node_base{
public:
    constexpr static node_type TYPE = T;
    static node_type get_type(){return TYPE;}

public:
    void set_id(long long id) override final{
        this->id_ = id;
    }
    long long get_id() override final {
        return this->id_;
    }
    result<T>& last_result() override final{
        return result_;
    }

    std::vector<InputPortBase *> get_input_port() override final{
        return input_ports_;
    }
private:
    long long id_ = 0;
    
protected:

    result<T> result_;
    std::vector<InputPortBase *> input_ports_;
};
}
