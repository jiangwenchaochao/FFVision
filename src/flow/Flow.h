#pragma once

// FFVision Flow Engine
// 统一头文件 - 包含所有流程引擎组件

#include "Node.h"
#include "Edge.h"
#include "Graph.h"
#include "ExecutionContext.h"
#include "FlowEngine.h"
#include "Nodes.h"

/*
 * 使用示例:
 * 
 * #include "flow/Flow.h"
 * using namespace flow;
 * 
 * int main() {
 *     // 创建图
 *     Graph graph;
 *     
 *     // 创建节点
 *     auto num1 = graph.createNode<NumberNode>("Number1", 10.0);
 *     auto num2 = graph.createNode<NumberNode>("Number2", 20.0);
 *     auto add = graph.createNode<AddNode>("Add");
 *     auto print = graph.createNode<PrintNode>("Print");
 *     
 *     // 连接节点
 *     graph.connect(num1, 0, add, 0);   // num1 -> add.a
 *     graph.connect(num2, 0, add, 1);   // num2 -> add.b
 *     graph.connect(add, 0, print, 0);  // add -> print
 *     
 *     // 执行
 *     FlowEngine engine;
 *     engine.run(graph);
 *     
 *     // 输出: Output: 30
 *     return 0;
 * }
 */
