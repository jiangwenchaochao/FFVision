#pragma once

#include <map>
#include <any>
#include <string>
#include <chrono>
#include <vector>
#include <functional>

namespace flow {

    // Execution record for a single node
    struct NodeExecutionRecord {
        int nodeId;
        std::string nodeName;
        bool success;
        std::string errorMessage;
        std::chrono::microseconds executionTime;
        std::map<int, std::any> outputs;
    };

    // Execution context - stores all intermediate data during execution
    class ExecutionContext {
    public:
        ExecutionContext() : totalExecutionTime_(0) {}
        
        // ========== Output Data Management ==========
        
        // Store node outputs
        void setOutputs(int nodeId, const std::map<int, std::any>& outputs) {
            nodeOutputs_[nodeId] = outputs;
        }
        
        // Get all outputs of a node
        std::map<int, std::any> getOutputs(int nodeId) const {
            auto it = nodeOutputs_.find(nodeId);
            return (it != nodeOutputs_.end()) ? it->second : std::map<int, std::any>{};
        }
        
        // Get output of a specific port
        std::any getOutput(int nodeId, int portIndex) const {
            auto it = nodeOutputs_.find(nodeId);
            if (it != nodeOutputs_.end()) {
                auto portIt = it->second.find(portIndex);
                if (portIt != it->second.end()) {
                    return portIt->second;
                }
            }
            return std::any{};
        }
        
        // Check if node has output
        bool hasOutput(int nodeId) const {
            return nodeOutputs_.find(nodeId) != nodeOutputs_.end();
        }
        
        // Type-safe output retrieval
        template<typename T>
        T getOutputAs(int nodeId, int portIndex, const T& defaultValue = T{}) const {
            auto output = getOutput(nodeId, portIndex);
            if (output.has_value()) {
                try {
                    return std::any_cast<T>(output);
                } catch (...) {
                    return defaultValue;
                }
            }
            return defaultValue;
        }
        
        // ========== Variable Storage (for loops etc.) ==========
        
        void setVariable(const std::string& name, const std::any& value) {
            variables_[name] = value;
        }
        
        std::any getVariable(const std::string& name) const {
            auto it = variables_.find(name);
            return (it != variables_.end()) ? it->second : std::any{};
        }
        
        template<typename T>
        T getVariableAs(const std::string& name, const T& defaultValue = T{}) const {
            auto var = getVariable(name);
            if (var.has_value()) {
                try {
                    return std::any_cast<T>(var);
                } catch (...) {
                    return defaultValue;
                }
            }
            return defaultValue;
        }
        
        bool hasVariable(const std::string& name) const {
            return variables_.find(name) != variables_.end();
        }
        
        // ========== Execution Records ==========
        
        void addRecord(const NodeExecutionRecord& record) {
            executionRecords_.push_back(record);
            totalExecutionTime_ += record.executionTime;
        }
        
        const std::vector<NodeExecutionRecord>& getRecords() const {
            return executionRecords_;
        }
        
        std::chrono::microseconds getTotalExecutionTime() const {
            return totalExecutionTime_;
        }
        
        // Get execution record for a specific node
        const NodeExecutionRecord* getRecord(int nodeId) const {
            for (const auto& record : executionRecords_) {
                if (record.nodeId == nodeId) {
                    return &record;
                }
            }
            return nullptr;
        }
        
        // ========== State Management ==========
        
        void clear() {
            nodeOutputs_.clear();
            variables_.clear();
            executionRecords_.clear();
            totalExecutionTime_ = std::chrono::microseconds(0);
        }
        
        // Clear only outputs, keep variables (for loops)
        void clearOutputs() {
            nodeOutputs_.clear();
        }

    private:
        std::map<int, std::map<int, std::any>> nodeOutputs_;  // nodeId -> (portIndex -> data)
        std::map<std::string, std::any> variables_;           // Named variables
        std::vector<NodeExecutionRecord> executionRecords_;   // Execution records
        std::chrono::microseconds totalExecutionTime_;
    };

} // namespace flow
