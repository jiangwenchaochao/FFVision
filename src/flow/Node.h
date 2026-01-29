#pragma once

#include "./Edge.h"

namespace node {
    enum class NodeCategory {
        Input,        // 输入源（Camera / Loader）
        Preprocess,   // 预处理
        Algorithm,    // 算法（内置 / 用户）
        Logic,        // 逻辑判断
        Output       // 输出
        //System        // 系统辅助
    };
    enum class NodeExecType {
        Source,      // 无输入（Camera）
        Transform,   // 输入→输出（Filter）
        Logic,    // 条件判断（If）
        Sink         // 只消费（Send / Save）
    };

    enum class NodeDataRole {
        Image,
        Region,
        Tuple,
        Scalar,
        Control,     // Trigger / Bool
        None         // 无数据（纯控制）
    };

    enum class NodeSideEffect {
        None,        // 纯函数
        IO,          // 相机 / PLC
        Network,     // TCP / UDP
        File,        // 保存 / 日志
        UI           // 显示
    };

    class Node {

    public:
        int get_id() = 0;
        char* get_name() = 0;
        ~Node() = 0; 

    private:
        int id_;
        std::vector<Edge*> input_edge;
        std::vector<Edge*> output_edge;
    };


}

