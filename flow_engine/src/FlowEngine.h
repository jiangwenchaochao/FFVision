#pragma once

#include "Graph.h"
#include "ExecutionContext.h"
#include <queue>
#include <set>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <cstdlib>

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

    struct PreNodeScriptConfig {
        bool enabled = false;
        std::string pythonPath = "python";
        std::string scriptText;
        std::string scriptPath;
    };

    struct UserScript {
        int id = 0;
        std::string name;
        std::string content;
        bool enabled = true;
        std::chrono::system_clock::time_point createdAt{};
        std::chrono::system_clock::time_point updatedAt{};
    };

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
        ExecuteResult executeNodeStandalone(NodePtr node, const std::map<std::string, std::any>& mockInputs) {
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
        void setPreNodeScriptText(const std::string& scriptText) {
            activeUserScriptId_ = -1;
            preNodeScriptConfig_.scriptText = scriptText;
            preNodeScriptConfig_.scriptPath.clear();
            preNodeScriptConfig_.enabled = !scriptText.empty();
        }
        void setPreNodeScriptPath(const std::string& scriptPath) {
            activeUserScriptId_ = -1;
            preNodeScriptConfig_.scriptPath = scriptPath;
            preNodeScriptConfig_.scriptText.clear();
            preNodeScriptConfig_.enabled = !scriptPath.empty();
        }
        void setPreNodeScriptEnabled(bool enabled) { preNodeScriptConfig_.enabled = enabled; }
        void setPreNodeScriptPythonPath(const std::string& pythonPath) {
            preNodeScriptConfig_.pythonPath = pythonPath.empty() ? "python" : pythonPath;
        }
        void clearPreNodeScript() {
            activeUserScriptId_ = -1;
            preNodeScriptConfig_ = PreNodeScriptConfig{};
        }

        // ========== User Script CRUD ==========

        int createUserScript(const std::string& name, const std::string& content, bool activate = false) {
            UserScript script;
            script.id = nextUserScriptId_++;
            script.name = name.empty() ? ("script_" + std::to_string(script.id)) : name;
            script.content = content;
            script.enabled = true;
            script.createdAt = std::chrono::system_clock::now();
            script.updatedAt = script.createdAt;
            userScripts_[script.id] = script;
            userScriptOrder_.push_back(script.id);

            if (activate) {
                setActiveUserScript(script.id);
            }
            return script.id;
        }

        std::vector<UserScript> listUserScripts() const {
            std::vector<UserScript> scripts;
            scripts.reserve(userScriptOrder_.size());
            for (int id : userScriptOrder_) {
                auto it = userScripts_.find(id);
                if (it != userScripts_.end()) {
                    scripts.push_back(it->second);
                }
            }
            return scripts;
        }

        bool getUserScript(int scriptId, UserScript& outScript) const {
            auto it = userScripts_.find(scriptId);
            if (it == userScripts_.end()) {
                return false;
            }
            outScript = it->second;
            return true;
        }

        bool updateUserScript(int scriptId, const std::string& name, const std::string& content) {
            auto it = userScripts_.find(scriptId);
            if (it == userScripts_.end()) {
                return false;
            }

            if (!name.empty()) {
                it->second.name = name;
            }
            it->second.content = content;
            it->second.updatedAt = std::chrono::system_clock::now();

            if (activeUserScriptId_ == scriptId) {
                preNodeScriptConfig_.scriptText = it->second.content;
                preNodeScriptConfig_.scriptPath.clear();
                preNodeScriptConfig_.enabled = it->second.enabled && !it->second.content.empty();
            }
            return true;
        }

        bool deleteUserScript(int scriptId) {
            auto it = userScripts_.find(scriptId);
            if (it == userScripts_.end()) {
                return false;
            }

            userScripts_.erase(it);
            userScriptOrder_.erase(
                std::remove(userScriptOrder_.begin(), userScriptOrder_.end(), scriptId),
                userScriptOrder_.end()
            );

            if (activeUserScriptId_ == scriptId) {
                activeUserScriptId_ = -1;
                preNodeScriptConfig_.enabled = false;
                preNodeScriptConfig_.scriptText.clear();
                preNodeScriptConfig_.scriptPath.clear();
            }
            return true;
        }

        bool setUserScriptEnabled(int scriptId, bool enabled) {
            auto it = userScripts_.find(scriptId);
            if (it == userScripts_.end()) {
                return false;
            }
            it->second.enabled = enabled;
            it->second.updatedAt = std::chrono::system_clock::now();

            if (activeUserScriptId_ == scriptId) {
                preNodeScriptConfig_.enabled = enabled && !it->second.content.empty();
            }
            return true;
        }

        bool setActiveUserScript(int scriptId) {
            auto it = userScripts_.find(scriptId);
            if (it == userScripts_.end()) {
                return false;
            }

            activeUserScriptId_ = scriptId;
            preNodeScriptConfig_.scriptPath.clear();
            preNodeScriptConfig_.scriptText = it->second.content;
            preNodeScriptConfig_.enabled = it->second.enabled && !it->second.content.empty();
            return true;
        }

        void clearActiveUserScript() {
            activeUserScriptId_ = -1;
            preNodeScriptConfig_.enabled = false;
            preNodeScriptConfig_.scriptText.clear();
            preNodeScriptConfig_.scriptPath.clear();
        }

        int getActiveUserScriptId() const { return activeUserScriptId_; }
        
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
        std::map<std::string, std::any> gatherInputs(NodePtr node, Graph& graph) {
            std::map<std::string, std::any> inputs;
            
            for (const auto& edge : graph.getIncomingEdges(node->getId())) {
                if (!edge->isActive()) continue;
                
                int srcId = edge->getSource()->getId();
                int srcPortIndex = edge->getSourcePort();
                int dstPortIndex = edge->getDestinationPort();
                
                // Get output from source node via context
                const std::string srcPortName = edge->getSource()->getOutputPortName(srcPortIndex);
                const std::string dstPortName = edge->getDestination()->getInputPortName(dstPortIndex);
                
                auto output = context_.getOutput(srcId, srcPortName);
                if (output.has_value()) {
                    inputs[dstPortName] = output;
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

            std::string preNodeError;
            if (!runPreNodeScript(node, inputs, preNodeError)) {
                // Observability callback must not block node execution.
                if (onError_) {
                    onError_(node->getId(), preNodeError);
                }
            }
            
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

        static std::string nodeTypeToString(NodeType type) {
            switch (type) {
                case NodeType::Source: return "Source";
                case NodeType::Transform: return "Transform";
                case NodeType::Logic: return "Logic";
                case NodeType::Loop: return "Loop";
                case NodeType::Sink: return "Sink";
            }
            return "Unknown";
        }

        static std::string quoteForShell(const std::string& s) {
            std::string escaped;
            escaped.reserve(s.size() + 2);
            escaped.push_back('"');
            for (char c : s) {
                if (c == '"') {
                    escaped += "\\\"";
                } else {
                    escaped.push_back(c);
                }
            }
            escaped.push_back('"');
            return escaped;
        }

        static std::string escapeValue(const std::string& v) {
            std::string out;
            out.reserve(v.size());
            for (char c : v) {
                if (c == '\\') out += "\\\\";
                else if (c == '\n') out += "\\n";
                else if (c == '\t') out += "\\t";
                else out.push_back(c);
            }
            return out;
        }

        static std::string unescapeValue(const std::string& v) {
            std::string out;
            out.reserve(v.size());
            for (size_t i = 0; i < v.size(); ++i) {
                if (v[i] == '\\' && i + 1 < v.size()) {
                    const char n = v[i + 1];
                    if (n == 'n') {
                        out.push_back('\n');
                        ++i;
                        continue;
                    }
                    if (n == 't') {
                        out.push_back('\t');
                        ++i;
                        continue;
                    }
                    if (n == '\\') {
                        out.push_back('\\');
                        ++i;
                        continue;
                    }
                }
                out.push_back(v[i]);
            }
            return out;
        }

        static bool writeAllText(const std::filesystem::path& path, const std::string& text) {
            std::ofstream ofs(path, std::ios::binary);
            if (!ofs) return false;
            ofs << text;
            return static_cast<bool>(ofs);
        }

        static std::string readAllText(const std::filesystem::path& path) {
            std::ifstream ifs(path, std::ios::binary);
            if (!ifs) return "";
            std::ostringstream oss;
            oss << ifs.rdbuf();
            return oss.str();
        }

        static bool writePreNodeInputFile(
            const std::filesystem::path& path,
            NodePtr node,
            const std::map<std::string, std::any>& inputs) {
            std::ofstream ofs(path, std::ios::binary);
            if (!ofs) return false;

            ofs << "M\tnode_id\tN\t" << node->getId() << "\n";
            ofs << "M\tnode_name\tS\t" << escapeValue(node->getName()) << "\n";
            ofs << "M\tnode_type\tS\t" << nodeTypeToString(node->getType()) << "\n";

            for (const auto& kv : inputs) {
                const std::string& key = kv.first;
                const auto& value = kv.second;
                if (value.type() == typeid(double)) {
                    ofs << "I\t" << key << "\tN\t" << std::any_cast<double>(value) << "\n";
                } else if (value.type() == typeid(int)) {
                    ofs << "I\t" << key << "\tN\t" << std::any_cast<int>(value) << "\n";
                } else if (value.type() == typeid(bool)) {
                    ofs << "I\t" << key << "\tB\t" << (std::any_cast<bool>(value) ? "1" : "0") << "\n";
                } else if (value.type() == typeid(std::string)) {
                    ofs << "I\t" << key << "\tS\t" << escapeValue(std::any_cast<std::string>(value)) << "\n";
                } else {
                    return false;
                }
            }
            return static_cast<bool>(ofs);
        }

        static std::string buildPreNodeRunnerScript() {
            return
                "import sys\n"
                "def _unescape(v: str) -> str:\n"
                "    return v.replace('\\\\t', '\\t').replace('\\\\n', '\\n').replace('\\\\\\\\', '\\\\')\n"
                "\n"
                "def _read_ctx(path):\n"
                "    meta = {}\n"
                "    inputs = {}\n"
                "    with open(path, 'r', encoding='utf-8') as f:\n"
                "        for line in f:\n"
                "            line = line.rstrip('\\n')\n"
                "            if not line:\n"
                "                continue\n"
                "            parts = line.split('\\t', 3)\n"
                "            if len(parts) != 4:\n"
                "                continue\n"
                "            kind, key, typ, raw = parts\n"
                "            if typ == 'N':\n"
                "                val = float(raw)\n"
                "            elif typ == 'B':\n"
                "                val = raw in ('1', 'true', 'True')\n"
                "            elif typ == 'S':\n"
                "                val = _unescape(raw)\n"
                "            else:\n"
                "                continue\n"
                "            if kind == 'M':\n"
                "                meta[key] = val\n"
                "            elif kind == 'I':\n"
                "                inputs[key] = val\n"
                "    return {\n"
                "        'node_id': int(meta.get('node_id', 0)),\n"
                "        'node_name': str(meta.get('node_name', '')),\n"
                "        'node_type': str(meta.get('node_type', '')),\n"
                "        'inputs': dict(inputs),\n"
                "    }\n"
                "\n"
                "def _load_callback(script_path):\n"
                "    src = open(script_path, 'r', encoding='utf-8').read()\n"
                "    safe_builtins = {\n"
                "        'abs': abs,\n"
                "        'min': min,\n"
                "        'max': max,\n"
                "        'sum': sum,\n"
                "        'len': len,\n"
                "        'range': range,\n"
                "        'int': int,\n"
                "        'float': float,\n"
                "        'str': str,\n"
                "        'bool': bool,\n"
                "        'round': round,\n"
                "        'print': print,\n"
                "    }\n"
                "    g = {'__builtins__': safe_builtins}\n"
                "    l = {}\n"
                "    exec(src, g, l)\n"
                "    fn = l.get('before_node') or g.get('before_node')\n"
                "    if not callable(fn):\n"
                "        raise RuntimeError('user script must define callable before_node(ctx)')\n"
                "    return fn\n"
                "\n"
                "def main():\n"
                "    if len(sys.argv) != 3:\n"
                "        return 2\n"
                "    script_path, in_path = sys.argv[1], sys.argv[2]\n"
                "    callback = _load_callback(script_path)\n"
                "    ctx = _read_ctx(in_path)\n"
                "    callback(ctx)\n"
                "    return 0\n"
                "\n"
                "if __name__ == '__main__':\n"
                "    raise SystemExit(main())\n";
        }

        bool runPreNodeScript(
            NodePtr node,
            const std::map<std::string, std::any>& inputs,
            std::string& err) {
            err.clear();

            if (!preNodeScriptConfig_.enabled) {
                return true;
            }

            std::string scriptText = preNodeScriptConfig_.scriptText;
            if (scriptText.empty() && !preNodeScriptConfig_.scriptPath.empty()) {
                std::ifstream ifs(preNodeScriptConfig_.scriptPath, std::ios::binary);
                if (!ifs) {
                    err = "cannot open pre-node scriptPath: " + preNodeScriptConfig_.scriptPath;
                    return false;
                }
                std::ostringstream oss;
                oss << ifs.rdbuf();
                scriptText = oss.str();
            }
            if (scriptText.empty()) {
                return true;
            }

            const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            const std::string token = std::to_string(node->getId()) + "_" + std::to_string(now);
            const std::filesystem::path tmpDir = std::filesystem::temp_directory_path() / "flow_engine_py_callback";
            std::error_code ec;
            std::filesystem::create_directories(tmpDir, ec);

            const std::filesystem::path runnerPath = tmpDir / ("runner_" + token + ".py");
            const std::filesystem::path userScriptPath = tmpDir / ("user_" + token + ".py");
            const std::filesystem::path inputPath = tmpDir / ("input_" + token + ".txt");
            const std::filesystem::path stderrPath = tmpDir / ("stderr_" + token + ".txt");

            auto cleanup = [&]() {
                std::filesystem::remove(runnerPath, ec);
                std::filesystem::remove(userScriptPath, ec);
                std::filesystem::remove(inputPath, ec);
                std::filesystem::remove(stderrPath, ec);
            };

            if (!writeAllText(runnerPath, buildPreNodeRunnerScript())) {
                err = "failed to write pre-node runner";
                cleanup();
                return false;
            }
            if (!writeAllText(userScriptPath, scriptText)) {
                err = "failed to write user pre-node script";
                cleanup();
                return false;
            }
            if (!writePreNodeInputFile(inputPath, node, inputs)) {
                err = "unsupported input type for pre-node script";
                cleanup();
                return false;
            }

            const std::string cmd =
                quoteForShell(preNodeScriptConfig_.pythonPath) + " -I -S " +
                quoteForShell(runnerPath.string()) + " " +
                quoteForShell(userScriptPath.string()) + " " +
                quoteForShell(inputPath.string()) +
                " 2> " + quoteForShell(stderrPath.string());

            const int rc = std::system(cmd.c_str());
            if (rc != 0) {
                const std::string stderrText = readAllText(stderrPath);
                if (!stderrText.empty()) {
                    err = stderrText;
                } else {
                    err = "python process failed";
                }
                cleanup();
                return false;
            }

            cleanup();
            return true;
        }
        
    private:
        EngineState state_;
        int currentNodeId_;
        std::string lastError_;
        ExecutionContext context_;
        std::set<int> breakpoints_;
        PreNodeScriptConfig preNodeScriptConfig_;
        std::map<int, UserScript> userScripts_;
        std::vector<int> userScriptOrder_;
        int nextUserScriptId_ = 1;
        int activeUserScriptId_ = -1;
        
        // Callback functions
        NodeCallback onNodeStart_;
        NodeCallback onNodeFinish_;
        ErrorCallback onError_;
        BreakpointCallback onBreakpoint_;
    };

} // namespace flow
