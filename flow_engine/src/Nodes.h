#pragma once

#include "Node.h"
#include "SourceImageNode.h"
#include <iostream>
#include <cmath>

namespace flow {

    // ========== Basic Node Examples ==========

    // Number constant node (Source type)
    class NumberNode : public Node {
    public:
        NumberNode(int id, const std::string& name, double value = 0.0)
            : Node(id, name, NodeType::Source) {
            setParam("value", value);
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return { PortInfo(0, "value", PortDataType::Number) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& /*inputs*/) override {
            double value = getParam<double>("value", 0.0);
            return ExecuteResult::Success({{"value", value}});
        }
    };

    // Add node (Transform type)
    class AddNode : public Node {
    public:
        AddNode(int id, const std::string& name)
            : Node(id, name, NodeType::Transform) {}
        
        std::vector<PortInfo> getInputPorts() const override {
            return {
                PortInfo(0, "a", PortDataType::Number),
                PortInfo(1, "b", PortDataType::Number)
            };
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return { PortInfo(0, "result", PortDataType::Number) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            try {
                double a = std::any_cast<double>(inputs.at("a"));
                double b = std::any_cast<double>(inputs.at("b"));
                return ExecuteResult::Success({{"result", a + b}});
            } catch (...) {
                return ExecuteResult::Error("Invalid input types for Add");
            }
        }
    };

    // Multiply node
    class MultiplyNode : public Node {
    public:
        MultiplyNode(int id, const std::string& name)
            : Node(id, name, NodeType::Transform) {}
        
        std::vector<PortInfo> getInputPorts() const override {
            return {
                PortInfo(0, "a", PortDataType::Number),
                PortInfo(1, "b", PortDataType::Number)
            };
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return { PortInfo(0, "result", PortDataType::Number) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            try {
                double a = std::any_cast<double>(inputs.at("a"));
                double b = std::any_cast<double>(inputs.at("b"));
                return ExecuteResult::Success({{"result", a * b}});
            } catch (...) {
                return ExecuteResult::Error("Invalid input types for Multiply");
            }
        }
    };

    // Compare node (returns boolean)
    class CompareNode : public Node {
    public:
        enum class Op { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual };
        
        CompareNode(int id, const std::string& name, Op op = Op::Greater)
            : Node(id, name, NodeType::Transform) {
            setParam("op", static_cast<int>(op));
        }
        
        std::vector<PortInfo> getInputPorts() const override {
            return {
                PortInfo(0, "a", PortDataType::Number),
                PortInfo(1, "b", PortDataType::Number)
            };
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return { PortInfo(0, "result", PortDataType::Bool) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            try {
                double a = std::any_cast<double>(inputs.at("a"));
                double b = std::any_cast<double>(inputs.at("b"));
                Op op = static_cast<Op>(getParam<int>("op", 0));
                
                bool result = false;
                switch (op) {
                    case Op::Equal:        result = (a == b); break;
                    case Op::NotEqual:     result = (a != b); break;
                    case Op::Less:         result = (a < b);  break;
                    case Op::LessEqual:    result = (a <= b); break;
                    case Op::Greater:      result = (a > b);  break;
                    case Op::GreaterEqual: result = (a >= b); break;
                }
                
                return ExecuteResult::Success({{"result", result}});
            } catch (...) {
                return ExecuteResult::Error("Invalid input types for Compare");
            }
        }
    };

    // ========== Control Flow Nodes ==========

    // IF conditional branch node
    class IfNode : public Node {
    public:
        IfNode(int id, const std::string& name)
            : Node(id, name, NodeType::Logic) {}
        
        std::vector<PortInfo> getInputPorts() const override {
            return {
                PortInfo(0, "condition", PortDataType::Bool),
                PortInfo(1, "data", PortDataType::Any, false)  // Optional data passthrough
            };
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return {
                PortInfo(0, "true", PortDataType::Any),   // True branch
                PortInfo(1, "false", PortDataType::Any)   // False branch
            };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            try {
                bool condition = std::any_cast<bool>(inputs.at("condition"));
                
                // Passthrough data (if any)
                std::any data;
                auto it = inputs.find("data");
                if (it != inputs.end()) {
                    data = it->second;
                }
                
                // Return branch selection
                int branch = condition ? 0 : 1;
                std::map<std::string, std::any> outputs;
                outputs[condition ? "true" : "false"] = data;
                
                return ExecuteResult::Branch(branch, outputs);
            } catch (...) {
                return ExecuteResult::Error("Invalid condition type for If");
            }
        }
    };

    // Switch multi-way branch node
    class SwitchNode : public Node {
    public:
        SwitchNode(int id, const std::string& name, int caseCount = 3)
            : Node(id, name, NodeType::Logic) {
            setParam("caseCount", caseCount);
        }
        
        std::vector<PortInfo> getInputPorts() const override {
            return {
                PortInfo(0, "index", PortDataType::Number),
                PortInfo(1, "data", PortDataType::Any, false)
            };
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            int count = getParam<int>("caseCount", 3);
            std::vector<PortInfo> ports;
            for (int i = 0; i < count; i++) {
                ports.emplace_back(i, "case" + std::to_string(i), PortDataType::Any);
            }
            return ports;
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            try {
                int index = static_cast<int>(std::any_cast<double>(inputs.at("index")));
                int caseCount = getParam<int>("caseCount", 3);
                
                // Clamp range
                if (index < 0) index = 0;
                if (index >= caseCount) index = caseCount - 1;
                
                std::any data;
                auto it = inputs.find("data");
                if (it != inputs.end()) {
                    data = it->second;
                }
                
                const std::string outKey = "case" + std::to_string(index);
                return ExecuteResult::Branch(index, {{outKey, data}});
            } catch (...) {
                return ExecuteResult::Error("Invalid index type for Switch");
            }
        }
    };

    // ========== Output Nodes ==========

    // Print node (Sink type)
    class PrintNode : public Node {
    public:
        PrintNode(int id, const std::string& name)
            : Node(id, name, NodeType::Sink) {}
        
        std::vector<PortInfo> getInputPorts() const override {
            return { PortInfo(0, "value", PortDataType::Any) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            auto it = inputs.find("value");
            if (it == inputs.end()) {
                return ExecuteResult::Error("No input for Print");
            }
            
            std::string prefix = getParam<std::string>("prefix", "Output");
            
            // Try printing different types
            try {
                if (it->second.type() == typeid(double)) {
                    std::cout << prefix << ": " << std::any_cast<double>(it->second) << std::endl;
                } else if (it->second.type() == typeid(int)) {
                    std::cout << prefix << ": " << std::any_cast<int>(it->second) << std::endl;
                } else if (it->second.type() == typeid(bool)) {
                    std::cout << prefix << ": " << (std::any_cast<bool>(it->second) ? "true" : "false") << std::endl;
                } else if (it->second.type() == typeid(std::string)) {
                    std::cout << prefix << ": " << std::any_cast<std::string>(it->second) << std::endl;
                } else {
                    std::cout << prefix << ": [value]" << std::endl;
                }
            } catch (...) {
                std::cout << prefix << ": [unknown type]" << std::endl;
            }
            
            return ExecuteResult::Success({});
        }
    };

    // ========== Utility Nodes ==========

    // Merge node (merge multiple inputs into one output, for IF branch convergence)
    class MergeNode : public Node {
    public:
        MergeNode(int id, const std::string& name, int inputCount = 2)
            : Node(id, name, NodeType::Transform) {
            setParam("inputCount", inputCount);
        }
        
        std::vector<PortInfo> getInputPorts() const override {
            int count = getParam<int>("inputCount", 2);
            std::vector<PortInfo> ports;
            for (int i = 0; i < count; i++) {
                ports.emplace_back(i, "in" + std::to_string(i), PortDataType::Any, false);
            }
            return ports;
        }
        
        std::vector<PortInfo> getOutputPorts() const override {
            return { PortInfo(0, "out", PortDataType::Any) };
        }
        
        ExecuteResult execute(const std::map<std::string, std::any>& inputs) override {
            // Return first input that has a value (in index order: in0, in1, ...)
            int count = getParam<int>("inputCount", 2);
            for (int i = 0; i < count; i++) {
                const std::string key = "in" + std::to_string(i);
                auto it = inputs.find(key);
                if (it != inputs.end() && it->second.has_value()) {
                    return ExecuteResult::Success({{"out", it->second}});
                }
            }
            return ExecuteResult::Success({});            
        }
    };

} // namespace flow
