#pragma once

#include <string>
#include <vector>
#include <map>
#include <any>
#include <memory>
#include <functional>

namespace flow {

    // Node type enumeration
    enum class NodeType {
        Source,      // No input (Camera, Image Reader)
        Transform,   // Input -> Output (Filter, Algorithm)
        Logic,       // Conditional (If, Switch)
        Loop,        // Loop control (While, For)
        Sink         // Output only (Send, Save, Display)
    };

    // Port data type
    enum class PortDataType {
        Any,         // Any type
        Image,       // Image
        Number,      // Numeric value
        String,      // String
        Bool,        // Boolean
        Array,       // Array
        Object       // Object
    };

    // Port definition
    struct PortInfo {
        int index;              // Port index
        std::string name;       // Port name
        PortDataType dataType;  // Data type
        bool required;          // Required (input port only)
        
        PortInfo(int idx, const std::string& n, PortDataType type, bool req = true)
            : index(idx), name(n), dataType(type), required(req) {}
    };

    // Node execution result
    struct ExecuteResult {
        bool success;
        std::string errorMessage;
        std::map<std::string, std::any> outputs;  // Port index -> Output data
        int nextBranch;  // For control flow nodes, -1 = default, 0/1 = branch selection
        
        ExecuteResult() : success(true), nextBranch(-1) {}
        
        static ExecuteResult Success(const std::map<std::string, std::any>& out) {
            ExecuteResult r;
            r.success = true;
            r.outputs = out;
            return r;
        }
        
        static ExecuteResult Error(const std::string& msg) {
            ExecuteResult r;
            r.success = false;
            r.errorMessage = msg;
            return r;
        }
        
        static ExecuteResult Branch(int branch, const std::map<std::string, std::any>& out = {}) {
            ExecuteResult r;
            r.success = true;
            r.nextBranch = branch;
            r.outputs = out;
            return r;
        }
    };

    // Node base class
    class Node {
    public:
        Node(int id, const std::string& name, NodeType type)
            : id_(id), name_(name), type_(type), active_(true) {}
        
        virtual ~Node() = default;

        // Core execution method - must be implemented by subclasses
        virtual ExecuteResult execute(const std::map<std::string, std::any>& inputs) = 0;
        
        // Get node input/output port definitions
        virtual std::vector<PortInfo> getInputPorts() const { return {}; }
        virtual std::vector<PortInfo> getOutputPorts() const { return {}; }
        
        // Node initialization (optional override)
        virtual bool init() { return true; }
        
        // Node reset (for resetting state before loops)
        virtual void reset() {}

        // Accessors
        int getId() const { return id_; }
        const std::string& getName() const { return name_; }
        NodeType getType() const { return type_; }
        
        // Active state (for conditional branching)
        bool isActive() const { return active_; }
        void setActive(bool active) { active_ = active; }
        
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
        NodeType type_;
        bool active_;
        std::map<std::string, std::any> params_;  // Node parameters
    };

    // Smart pointer type alias
    using NodePtr = std::shared_ptr<Node>;

} // namespace flow
