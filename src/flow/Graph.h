#pragma once

#include "Node.h"
#include "Edge.h"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <stdexcept>

namespace flow {

    // Graph class - manages collection of nodes and edges
    class Graph {
    public:
        Graph() : nextNodeId_(1), nextEdgeId_(1) {}
        
        // ========== Node Management ==========
        
        // Add node (takes ownership)
        void addNode(NodePtr node) {
            if (!node) return;
            int id = node->getId();
            nodes_[id] = node;
            nodeOrder_.push_back(id);
        }
        
        // Create and add node
        template<typename T, typename... Args>
        std::shared_ptr<T> createNode(const std::string& name, Args&&... args) {
            auto node = std::make_shared<T>(nextNodeId_++, name, std::forward<Args>(args)...);
            addNode(node);
            return node;
        }
        
        // Remove node (also removes connected edges)
        void removeNode(int nodeId) {
            // Remove all edges connected to this node
            edges_.erase(
                std::remove_if(edges_.begin(), edges_.end(),
                    [nodeId](const EdgePtr& e) {
                        return e->getSource()->getId() == nodeId || 
                               e->getDestination()->getId() == nodeId;
                    }),
                edges_.end()
            );
            
            // Remove node
            nodes_.erase(nodeId);
            nodeOrder_.erase(
                std::remove(nodeOrder_.begin(), nodeOrder_.end(), nodeId),
                nodeOrder_.end()
            );
        }
        
        // Get node by ID
        NodePtr getNode(int id) const {
            auto it = nodes_.find(id);
            return (it != nodes_.end()) ? it->second : nullptr;
        }
        
        // Get all nodes
        std::vector<NodePtr> getNodes() const {
            std::vector<NodePtr> result;
            for (int id : nodeOrder_) {
                auto it = nodes_.find(id);
                if (it != nodes_.end()) {
                    result.push_back(it->second);
                }
            }
            return result;
        }
        
        // ========== Edge Management ==========
        
        // Add edge (connect two nodes)
        EdgePtr connect(int srcNodeId, int srcPort, int dstNodeId, int dstPort) {
            Node* src = getNode(srcNodeId).get();
            Node* dst = getNode(dstNodeId).get();
            
            if (!src || !dst) {
                throw std::runtime_error("Invalid node ID for connection");
            }
            
            auto edge = std::make_shared<Edge>(nextEdgeId_++, src, srcPort, dst, dstPort);
            edges_.push_back(edge);
            return edge;
        }
        
        // Add edge (using node pointers)
        EdgePtr connect(NodePtr src, int srcPort, NodePtr dst, int dstPort) {
            if (!src || !dst) {
                throw std::runtime_error("Null node for connection");
            }
            return connect(src->getId(), srcPort, dst->getId(), dstPort);
        }
        
        // Disconnect edge
        void disconnect(int edgeId) {
            edges_.erase(
                std::remove_if(edges_.begin(), edges_.end(),
                    [edgeId](const EdgePtr& e) { return e->getId() == edgeId; }),
                edges_.end()
            );
        }
        
        // Get all edges
        const std::vector<EdgePtr>& getEdges() const {
            return edges_;
        }
        
        // Get incoming edges (edges connecting to this node)
        std::vector<EdgePtr> getIncomingEdges(int nodeId) const {
            std::vector<EdgePtr> result;
            for (const auto& edge : edges_) {
                if (edge->getDestination()->getId() == nodeId && edge->isActive()) {
                    result.push_back(edge);
                }
            }
            return result;
        }
        
        // Get outgoing edges (edges from this node)
        std::vector<EdgePtr> getOutgoingEdges(int nodeId) const {
            std::vector<EdgePtr> result;
            for (const auto& edge : edges_) {
                if (edge->getSource()->getId() == nodeId && edge->isActive()) {
                    result.push_back(edge);
                }
            }
            return result;
        }
        
        // ========== Graph Analysis ==========
        
        // Get in-degree (number of incoming edges)
        int getInDegree(int nodeId) const {
            int count = 0;
            for (const auto& edge : edges_) {
                if (edge->getDestination()->getId() == nodeId && edge->isActive()) {
                    count++;
                }
            }
            return count;
        }
        
        // Get out-degree (number of outgoing edges)
        int getOutDegree(int nodeId) const {
            int count = 0;
            for (const auto& edge : edges_) {
                if (edge->getSource()->getId() == nodeId && edge->isActive()) {
                    count++;
                }
            }
            return count;
        }
        
        // Get source nodes (nodes with no inputs)
        std::vector<NodePtr> getSourceNodes() const {
            std::vector<NodePtr> result;
            for (const auto& [id, node] : nodes_) {
                if (getInDegree(id) == 0) {
                    result.push_back(node);
                }
            }
            return result;
        }
        
        // Get sink nodes (nodes with no outputs)
        std::vector<NodePtr> getSinkNodes() const {
            std::vector<NodePtr> result;
            for (const auto& [id, node] : nodes_) {
                if (getOutDegree(id) == 0) {
                    result.push_back(node);
                }
            }
            return result;
        }
        
        // Clear graph
        void clear() {
            nodes_.clear();
            edges_.clear();
            nodeOrder_.clear();
            nextNodeId_ = 1;
            nextEdgeId_ = 1;
        }
        
        // Node count
        size_t nodeCount() const { return nodes_.size(); }
        
        // Edge count
        size_t edgeCount() const { return edges_.size(); }

    private:
        std::unordered_map<int, NodePtr> nodes_;  // Node ID -> Node
        std::vector<EdgePtr> edges_;               // All edges
        std::vector<int> nodeOrder_;               // Node insertion order
        int nextNodeId_;
        int nextEdgeId_;
    };

} // namespace flow
