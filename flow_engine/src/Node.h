#pragma once
#include "context.h"
#include "result.h"
#include "node_type.h"
#include "result.h"

namespace flow{

// 端口元信息
struct PortInfo {
    int    index;
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

#define BEGIN_INPUT_PORTS()                                                  \
public:                                                                     \
    /* 返回端口元信息列表 */                                                   \
    static const std::vector<flow::PortInfo>& inputPortsInfo() {            \
        static const std::vector<flow::PortInfo> infos = {

#define INPUT_PORT(Index, Type, Name)                                       \
            flow::PortInfo{ Index, #Name, #Type },

#define END_INPUT_PORTS()                                                   \
        };                                                                  \
        return infos;                                                       \
    }                                                                       \
    static size_t inputPortCount() {                                        \
        return inputPortsInfo().size();                                     \
    }                                                                       \
    /* 通过索引获取输入端口（类型擦除） */                                       \
    flow::InputPortBase* inputPort(int index) {                             \
        auto it = input_ports_map_.find(index);                             \
        return it != input_ports_map_.end() ? it->second : nullptr;         \
    }                                                                       \
private:                                                                    \
    std::unordered_map<int, flow::InputPortBase*> input_ports_map_;         \
                                                                            \
    /* 子类构造时调用，把成员端口注册到 map */                                  \
    void registerInputPorts_()

// 在构造函数体或 init() 中注册端口实例
#define REGISTER_INPUT(Index, PortMember)                                   \
    input_ports_map_[Index] = &PortMember


class node_base{
public:
    ~node_base(){};
    virtual void init() = 0;
    virtual void stop() = 0;
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
    long long get_id() const override final{
        return this->id_;
    }
    result<Type>& last_result() override final{
        return result_;
    }
private:
    long long id_ = 0;
    std::vector<node_base *> child_nodes_;
    
protected:

    result<Type> result_;
};
}
