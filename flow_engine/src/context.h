#include <string>
#include <any>

namespace flow{
class osd;

/// 算法上下文接口
class context {
public:
    virtual ~context() {}

    /// 传入的图像
    // virtual cv::Mat & mat() = 0;

    /// 传入的OSD对象
    virtual osd & osd() = 0;

    virtual void set(const std::string & name, const std::any & value) = 0;
    virtual void set(const std::string & name, std::any && value) = 0;

    /// 获取上下文变量
    virtual const std::any & get(const std::string & name) const = 0;

    /// 判断是否存在指定的上下文变量
    virtual bool has(const std::string & name) const = 0;

    /// 获取上下文变量
    template<class T>
    T get_value(const std::string & name) const {
        return has(name) ? std::any_cast<T>(get(name)) : T{};
    }
};

}