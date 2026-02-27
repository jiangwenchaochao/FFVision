#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <memory>
#include <functional>
#include <typeindex>
#include <type_traits>
#include <cstdint>
#include "context.h"

namespace flow {

    /// 结果类型
    enum class result {
        ok,         
        ng_low,     
        ng_high,   
        ng_error,   
        retest      
    };

    // Node type enumeration
    enum class NodeType {
        Source,      // No input (Camera, Image Reader)
        Transform,   // Input -> Output (Filter, Algorithm)
        Logic,       // Conditional (If, Switch)
        Loop,        // Loop control (While, For)
        Sink         // Output only (Send, Save, Display)
    };

    // Port data type (high-level category for UI and basic validation)
    enum class PortDataType {
        Any,
        Image,
        Binary,
        Number,
        String,
        Bool,
        Array,
        Object
    };

    // Forward declaration for OpenCV type without including headers here.
    namespace cv {
        class Mat;
    }

    // Port definition
    struct PortInfo {
        int index = 0;
        std::string name;
        PortDataType dataType = PortDataType::Any;
        bool required = true;
        std::string valueType; // optional C++ type name, e.g. "cv::Mat"
        std::string desc;
        std::type_index cppType = typeid(void); // optional runtime type checking

        PortInfo() = default;

        PortInfo(int idx,
                 const std::string& portName,
                 PortDataType t,
                 bool req = true,
                 const std::string& vt = {},
                 const std::string& d = {})
            : index(idx)
            , name(portName)
            , dataType(t)
            , required(req)
            , valueType(vt)
            , desc(d)
            , cppType(typeid(void)) {}

        PortInfo(int idx,
                 const std::string& portName,
                 PortDataType t,
                 bool req,
                 const std::string& vt,
                 const std::string& d,
                 std::type_index type)
            : index(idx)
            , name(portName)
            , dataType(t)
            , required(req)
            , valueType(vt)
            , desc(d)
            , cppType(type) {}

        template<typename T>
        static PortInfo Typed(int idx,
                              const std::string& portName,
                              bool req = true,
                              const std::string& d = {},
                              const std::string& vtOverride = {}) {
            const std::string vt = vtOverride.empty() ? PortTypeTraits<T>::valueType() : vtOverride;
            return PortInfo(idx, portName, PortTypeTraits<T>::dataType, req, vt, d, typeid(T));
        }
    };

    // Node execution result
    struct NodeResult {
        result result;
        std::string msg;
        NodeResult(){}
        
        static NodeResult Success(const std::map<std::string, std::any>& out) {
            NodeResult r;
            r.result = result::ok;
            return r;
        }
        
        static NodeResult Error(const std::string& msg) {
            NodeResult r;
            r.success = false;
            r.errorMessage = msg;
            return r;
        }
    };

    class Node {
    public:
        virtual ~Node(){};

        // Core execution method - must be implemented by subclasses
        virtual NodeResult execute(context & context) = 0;
        
        // Get node input/output port definitions
        virtual std::vector<PortInfo> getInputPorts() const { return input_ports_; }
        virtual std::vector<PortInfo> getOutputPorts() const { return output_ports_; }

        // Node initialization (optional override)
        virtual bool init() { return true; }
        
        // Node reset (for resetting state before loops)
        virtual void reset() {}

        // Accessors
        int getId() const { return id_; }
        const std::string& getName() const { return name_; }
        
        const PortInfo* getOutputPortInfo(int index) const {
            for (const PortInfo& p : output_ports_) {
                if (p.index == index) return &p;
            }
            return nullptr;
        }
        const PortInfo* getInputPortInfo(int index) const {
            for (const PortInfo& p : input_ports_) {
                if (p.index == index) return &p;
            }
            return nullptr;
        }

        std::string getOutputPortName(int index) const {
            for (const PortInfo& outputPort : output_ports_) {
                if (outputPort.index == index) {
                    return outputPort.name;
                }
            }
            return "";
        }
        std::string getInputPortName(int index) const {
            for (const PortInfo& inputPort : input_ports_) {
                if (inputPort.index == index) {
                    return inputPort.name;
                }
            }
            return "";
        }

        // // Active state (for conditional branching)
        // bool isActive() const { return active_; }
        // void setActive(bool active) { active_ = active; }
        
        // Parameter system
        void setParam(const std::string& key, const std::any& value) {
            params_[key] = value;
        }
        
        template<typename T>
        T getParam(const std::string& key, const T& defaultValue = T{}) const {
            auto it = params_.find(key);
            if (it != params_.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (...) {
                    return defaultValue;
                }
            }
            return defaultValue;
        }
        
        bool hasParam(const std::string& key) const {
            return params_.find(key) != params_.end();
        }

    protected:
        int id_;
        std::string name_;
        std::vector<PortInfo> input_ports_;
        std::vector<PortInfo> output_ports_;
        std::map<std::string, std::any> params_;  // Node parameters

        // Helpers for derived nodes to declare typed ports via templates.
        template<typename T>
        void addInputPort(int index,
                          const std::string& portName,
                          bool required = true,
                          const std::string& desc = {},
                          const std::string& valueTypeOverride = {}) {
            input_ports_.push_back(PortInfo::Typed<T>(index, portName, required, desc, valueTypeOverride));
        }

        template<typename T>
        void addOutputPort(int index,
                           const std::string& portName,
                           bool required = true,
                           const std::string& desc = {},
                           const std::string& valueTypeOverride = {}) {
            output_ports_.push_back(PortInfo::Typed<T>(index, portName, required, desc, valueTypeOverride));
        }
    };

    // Smart pointer type alias
    using NodePtr = std::shared_ptr<Node>;

} // namespace flow
