#pragma once

#include <memory>

namespace flow {

    // Forward declaration
    class Node;

    // Edge class - represents data flow between nodes
    class Edge {
    public:
        Edge(int id, Node* src, int srcPort, Node* dst, int dstPort)
            : id_(id)
            , src_(src)
            , dst_(dst)
            , srcOutputPort_(srcPort)
            , dstInputPort_(dstPort)
            , active_(true) {}
        
        // Accessors
        int getId() const { return id_; }
        
        Node* getSource() const { return src_; }
        Node* getDestination() const { return dst_; }
        
        int getSourcePort() const { return srcOutputPort_; }
        int getDestinationPort() const { return dstInputPort_; }
        
        // Active state (for disabling edges in conditional branches)
        bool isActive() const { return active_; }
        void setActive(bool active) { active_ = active; }

    private:
        int id_;
        Node* src_;           // Source node (data producer)
        Node* dst_;           // Destination node (data consumer)
        int srcOutputPort_;   // Source node output port index
        int dstInputPort_;    // Destination node input port index
        bool active_;         // Whether edge is active
    };

    // Smart pointer type alias
    using EdgePtr = std::shared_ptr<Edge>;

} // namespace flow
