#pragma once

#include "Graph.h"
#include "ExecutionContext.h"
#include <queue>
#include <set>
#include <functional>
#include <stdexcept>
#include <chrono>

namespace flow {

    // Execution state
    enum class EngineState {
        Idle,       // Idle
        Running,    // Running
        Paused,     // Paused (breakpoint)
        Finished,   // Finished
        Error       // Error
    };

    // Execution options
    struct ExecutionOptions {
        int maxIterations = 10000;       // Max iterations (prevent infinite loops)
        bool stopOnError = true;         // Stop on error
        bool recordExecutionTime = true; // Record execution time
    };

    // Callback function types
    using NodeCallback = std::function<void(int nodeId, const std::string& nodeName)>;
    using ErrorCallback = std::function<void(int nodeId, const std::string& error)>;
    using BreakpointCallback = std::function<void(int nodeId)>;

    // Flow execution engine
    class FlowEngine {
    public:
        FlowEngine() : state_(EngineState::Idle), currentNodeId_(-1) {}
        
        // ========== Main Execution Methods ==========
        
        // Execute entire graph
        bool run(Graph& graph, const ExecutionOptions& options = ExecutionOptions{}) {
            state_ = EngineState::Running;
            context_.clear();
            
            // Reset all node states
            for (auto& node : graph.getNodes()) {
                node->setActive(true);
                node->reset();
            }
            
            // Reset all edge states
            for (auto& edge : graph.getEdges()) {
                edge->setActive(true);
            }
            
            // Topological sort
            std::vector<NodePtr> sortedNodes;
            if (!topologicalSort(graph, sortedNodes)) {
                lastError_ = "Graph contains cycles (use Loop nodes for iteration)";
                state_ = EngineState::Error;
                return false;
            }
            
            // Execute nodes in order
            for (const auto& node : sortedNodes) {
                if (!executeNode(node, graph, options)) {
                    if (options.stopOnError) {
                        state_ = EngineState::Error;
                        return false;
                    }
                }
                
                // Check if paused (breakpoint)
                if (state_ == EngineState::Paused) {
                    return true;  // Paused but not error
                }
            }
            
            state_ = EngineState::Finished;
            return true;
        }
        
        // Execute until specified node
        bool runUntil(Graph& graph, int targetNodeId, const ExecutionOptions& options = ExecutionOptions{}) {
            state_ = EngineState::Running;
            context_.clear();
            
            std::vector<NodePtr> sortedNodes;
            if (!topologicalSort(graph, sortedNodes)) {
                lastError_ = "Graph contains cycles";
                state_ = EngineState::Error;
                return false;
            }
            
            for (const auto& node : sortedNodes) {
                if (!executeNode(node, graph, options)) {
                    if (options.stopOnError) {
                        state_ = EngineState::Error;
                        return false;
                    }
                }
                
                if (node->getId() == targetNodeId) {
                    state_ = EngineState::Paused;
                    return true;
                }
            }
            
            state_ = EngineState::Finished;
            return true;
        }
        
        // Execute single node standalone (for debugging)
        ExecuteResult executeNodeStandalone(NodePtr node, const std::map<int, std::any>& mockInputs) {
            if (!node) {
                return ExecuteResult::Error("Null node");
            }
            return node->execute(mockInputs);
        }
        
        // Resume execution from current position
        bool resume(Graph& graph, const ExecutionOptions& options = ExecutionOptions{}) {
            if (state_ != EngineState::Paused) {
                return false;
            }
            
            std::vector<NodePtr> sortedNodes;
            if (!topologicalSort(graph, sortedNodes)) {
                return false;
            }
            
            // Find nodes after current and continue execution
            bool foundCurrent = false;
            for (const auto& node : sortedNodes) {
                if (node->getId() == currentNodeId_) {
                    foundCurrent = true;
                    continue;
                }
                
                if (foundCurrent) {
                    if (!executeNode(node, graph, options)) {
                        if (options.stopOnError) {
                            state_ = EngineState::Error;
                            return false;
                        }
                    }
                    
                    if (state_ == EngineState::Paused) {
                        return true;
                    }
                }
            }
            
            state_ = EngineState::Finished;
            return true;
        }
        
        // Step over (execute next node)
        bool stepOver(Graph& graph, const ExecutionOptions& options = ExecutionOptions{}) {
            std::vector<NodePtr> sortedNodes;
            if (!topologicalSort(graph, sortedNodes)) {
                return false;
            }
            
            // Find next node to execute
            bool foundCurrent = (currentNodeId_ == -1);  // Start from beginning if no current node
            for (const auto& node : sortedNodes) {
                if (!foundCurrent) {
                    if (node->getId() == currentNodeId_) {
                        foundCurrent = true;
                    }
                    continue;
                }
                
                // Skip inactive nodes
                if (!node->isActive()) continue;
                
                state_ = EngineState::Running;
                if (!executeNode(node, graph, options)) {
                    state_ = EngineState::Error;
                    return false;
                }
                
                state_ = EngineState::Paused;
                return true;
            }
            
            state_ = EngineState::Finished;
            return true;
        }
        
        // ========== Breakpoint Management ==========
        
        void addBreakpoint(int nodeId) {
            breakpoints_.insert(nodeId);
        }
        
        void removeBreakpoint(int nodeId) {
            breakpoints_.erase(nodeId);
        }
        
        void clearBreakpoints() {
            breakpoints_.clear();
        }
        
        bool hasBreakpoint(int nodeId) const {
            return breakpoints_.find(nodeId) != breakpoints_.end();
        }
        
        const std::set<int>& getBreakpoints() const {
            return breakpoints_;
        }
        
        // ========== Callback Settings ==========
        
        void setOnNodeStart(NodeCallback callback) { onNodeStart_ = callback; }
        void setOnNodeFinish(NodeCallback callback) { onNodeFinish_ = callback; }
        void setOnError(ErrorCallback callback) { onError_ = callback; }
        void setOnBreakpoint(BreakpointCallback callback) { onBreakpoint_ = callback; }
        
        // ========== State Query ==========
        
        EngineState getState() const { return state_; }
        int getCurrentNodeId() const { return currentNodeId_; }
        const std::string& getLastError() const { return lastError_; }
        ExecutionContext& getContext() { return context_; }
        const ExecutionContext& getContext() const { return context_; }
        
    private:
        // Topological sort (Kahn's algorithm)
        bool topologicalSort(Graph& graph, std::vector<NodePtr>& result) {
            result.clear();
            
            // Calculate in-degree for all nodes
            std::map<int, int> inDegree;
            for (const auto& node : graph.getNodes()) {
                inDegree[node->getId()] = 0;
            }
            
            for (const auto& edge : graph.getEdges()) {
                if (edge->isActive()) {
                    inDegree[edge->getDestination()->getId()]++;
                }
            }
            
            // Add nodes with in-degree 0 to queue
            std::queue<NodePtr> queue;
            for (const auto& node : graph.getNodes()) {
                if (inDegree[node->getId()] == 0) {
                    queue.push(node);
                }
            }
            
            // BFS
            while (!queue.empty()) {
                NodePtr node = queue.front();
                queue.pop();
                result.push_back(node);
                
                // Decrease in-degree of successor nodes
                for (const auto& edge : graph.getOutgoingEdges(node->getId())) {
                    int dstId = edge->getDestination()->getId();
                    inDegree[dstId]--;
                    if (inDegree[dstId] == 0) {
                        queue.push(graph.getNode(dstId));
                    }
                }
            }
            
            // If result count != node count, graph has cycles
            return result.size() == graph.nodeCount();
        }
        
        // Gather input data for a node
        std::map<int, std::any> gatherInputs(NodePtr node, Graph& graph) {
            std::map<int, std::any> inputs;
            
            for (const auto& edge : graph.getIncomingEdges(node->getId())) {
                if (!edge->isActive()) continue;
                
                int srcId = edge->getSource()->getId();
                int srcPort = edge->getSourcePort();
                int dstPort = edge->getDestinationPort();
                
                // Get output from source node via context
                auto output = context_.getOutput(srcId, srcPort);
                if (output.has_value()) {
                    inputs[dstPort] = output;
                }
            }
            
            return inputs;
        }
        
        // Execute single node
        bool executeNode(NodePtr node, Graph& graph, const ExecutionOptions& options) {
            if (!node || !node->isActive()) {
                return true;  // Skip inactive nodes
            }
            
            currentNodeId_ = node->getId();
            
            // Check breakpoint
            if (hasBreakpoint(node->getId())) {
                state_ = EngineState::Paused;
                if (onBreakpoint_) {
                    onBreakpoint_(node->getId());
                }
                return true;
            }
            
            // Callback: node start
            if (onNodeStart_) {
                onNodeStart_(node->getId(), node->getName());
            }
            
            // Gather inputs
            auto inputs = gatherInputs(node, graph);
            
            // Execute node
            auto startTime = std::chrono::high_resolution_clock::now();
            ExecuteResult result = node->execute(inputs);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            // Record execution info
            if (options.recordExecutionTime) {
                NodeExecutionRecord record;
                record.nodeId = node->getId();
                record.nodeName = node->getName();
                record.success = result.success;
                record.errorMessage = result.errorMessage;
                record.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
                record.outputs = result.outputs;
                context_.addRecord(record);
            }
            
            if (!result.success) {
                lastError_ = "Node [" + node->getName() + "]: " + result.errorMessage;
                if (onError_) {
                    onError_(node->getId(), result.errorMessage);
                }
                return false;
            }
            
            // Store outputs
            context_.setOutputs(node->getId(), result.outputs);
            
            // Handle control flow (conditional branching)
            if (result.nextBranch >= 0) {
                handleBranching(node, graph, result.nextBranch);
            }
            
            // Callback: node finish
            if (onNodeFinish_) {
                onNodeFinish_(node->getId(), node->getName());
            }
            
            return true;
        }
        
        // Handle conditional branching
        void handleBranching(NodePtr node, Graph& graph, int selectedBranch) {
            // Get all outgoing edges from this node
            auto outEdges = graph.getOutgoingEdges(node->getId());
            
            for (auto& edge : outEdges) {
                // Activate/deactivate edges based on branch selection
                // Convention: port 0 = True branch, port 1 = False branch
                bool shouldActivate = (edge->getSourcePort() == selectedBranch);
                edge->setActive(shouldActivate);
                
                // Recursively deactivate downstream nodes of inactive branch
                if (!shouldActivate) {
                    deactivateDownstream(edge->getDestination()->getId(), graph);
                }
            }
        }
        
        // Recursively deactivate downstream nodes
        void deactivateDownstream(int nodeId, Graph& graph) {
            auto node = graph.getNode(nodeId);
            if (!node) return;
            
            node->setActive(false);
            
            for (auto& edge : graph.getOutgoingEdges(nodeId)) {
                edge->setActive(false);
                deactivateDownstream(edge->getDestination()->getId(), graph);
            }
        }
        
    private:
        EngineState state_;
        int currentNodeId_;
        std::string lastError_;
        ExecutionContext context_;
        std::set<int> breakpoints_;
        
        // Callback functions
        NodeCallback onNodeStart_;
        NodeCallback onNodeFinish_;
        ErrorCallback onError_;
        BreakpointCallback onBreakpoint_;
    };

} // namespace flow
