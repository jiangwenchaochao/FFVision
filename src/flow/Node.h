#pragma once

#include "./Edge.h"

namespace flow {

    enum class NodeType {
        Source,      // 无输入（Camera）
        Transform,   // 输入→输出（Filter）
        Logic,    // 条件判断（If）
        Sink         // 只消费（Send / Save）
    };

    //enum class NodeDataRole {
    //    Image,
    //    Region,
    //    Tuple,
    //    Scalar,
    //    Control,     // Trigger / Bool
    //    None         // 无数据（纯控制）
    //};

    //enum class NodeSideEffect {
    //    None,        // 纯函数
    //    IO,          // 相机 / PLC
    //    Network,     // TCP / UDP
    //    File,        // 保存 / 日志
    //    UI           // 显示
    //};

    <template NodeType node_type>
    class Node {
        constexpr static  NodeType nodeType = node_type
    public:
        Node(int id, char* name) {
            id_ = id;
            name_ = name;
        }
        bool init() {
            return true;
        }
        int get_id() {
            return id_;
        };
        char* get_name() {
            return name_;
        };
        NodeType get_type() {
            return nodeType;
        }

        ~Node() {
        };

    private:
        int id_;
        char* name_;
    };


    class NodeFactory {

    public:
        NodeFactory() {
        
        }
        static NodeFactory* getInstance() {
            return instance_;
        }
    public:
        Node* getNode(NodeType type, int id, char* name) {
            
        }

    private:
        static NodeFactory instance_;
    };

}

