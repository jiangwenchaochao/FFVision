// FlowEngine 使用示例
// 编译: 需要 C++17 支持

#include "Flow.h"
#include <iostream>

using namespace flow;

// 示例1: 简单的数学计算
void example_basic() {
    std::cout << "=== Example 1: Basic Math ===" << std::endl;
    
    Graph graph;
    
    // 创建节点: (10 + 20) * 3 = 90
    auto num1 = graph.createNode<NumberNode>("Number1", 10.0);
    auto num2 = graph.createNode<NumberNode>("Number2", 20.0);
    auto num3 = graph.createNode<NumberNode>("Number3", 3.0);
    auto add = graph.createNode<AddNode>("Add");
    auto mul = graph.createNode<MultiplyNode>("Multiply");
    auto print = graph.createNode<PrintNode>("Print");
    print->setParam("prefix", "Result");
    
    // 连接
    graph.connect(num1, 0, add, 0);   // 10 -> add.a
    graph.connect(num2, 0, add, 1);   // 20 -> add.b
    graph.connect(add, 0, mul, 0);    // (10+20) -> mul.a
    graph.connect(num3, 0, mul, 1);   // 3 -> mul.b
    graph.connect(mul, 0, print, 0);  // result -> print
    
    // 执行
    FlowEngine engine;
    engine.run(graph);
    
    std::cout << std::endl;
}

// 示例2: IF 条件分支
void example_if_branch() {
    std::cout << "=== Example 2: IF Branch ===" << std::endl;
    
    Graph graph;
    
    // 场景: 如果 a > b，输出 "True"，否则输出 "False"
    auto a = graph.createNode<NumberNode>("A", 15.0);
    auto b = graph.createNode<NumberNode>("B", 10.0);
    auto compare = graph.createNode<CompareNode>("Compare", CompareNode::Op::Greater);
    auto ifNode = graph.createNode<IfNode>("If");
    
    auto printTrue = graph.createNode<PrintNode>("PrintTrue");
    printTrue->setParam("prefix", "True Branch");
    
    auto printFalse = graph.createNode<PrintNode>("PrintFalse");
    printFalse->setParam("prefix", "False Branch");
    
    // 连接
    graph.connect(a, 0, compare, 0);        // a -> compare.a
    graph.connect(b, 0, compare, 1);        // b -> compare.b
    graph.connect(compare, 0, ifNode, 0);   // result -> if.condition
    graph.connect(a, 0, ifNode, 1);         // a -> if.data (透传数据)
    graph.connect(ifNode, 0, printTrue, 0); // if.true -> printTrue
    graph.connect(ifNode, 1, printFalse, 0);// if.false -> printFalse
    
    // 执行
    FlowEngine engine;
    engine.run(graph);
    
    std::cout << std::endl;
}

// 示例3: 调试功能 - 断点和单步执行
void example_debug() {
    std::cout << "=== Example 3: Debug Features ===" << std::endl;
    
    Graph graph;
    
    auto num1 = graph.createNode<NumberNode>("Number1", 5.0);
    auto num2 = graph.createNode<NumberNode>("Number2", 3.0);
    auto add = graph.createNode<AddNode>("Add");
    auto print = graph.createNode<PrintNode>("Print");
    
    graph.connect(num1, 0, add, 0);
    graph.connect(num2, 0, add, 1);
    graph.connect(add, 0, print, 0);
    
    FlowEngine engine;
    
    // 设置回调
    engine.setOnNodeStart([](int id, const std::string& name) {
        std::cout << "[START] Node " << id << ": " << name << std::endl;
    });
    
    engine.setOnNodeFinish([](int id, const std::string& name) {
        std::cout << "[FINISH] Node " << id << ": " << name << std::endl;
    });
    
    // 在 Add 节点设置断点
    engine.addBreakpoint(add->getId());
    
    engine.setOnBreakpoint([&](int id) {
        std::cout << "[BREAKPOINT] Hit at node " << id << std::endl;
        std::cout << "  Current outputs in context:" << std::endl;
        
        // 查看已执行节点的输出
        for (auto& node : graph.getNodes()) {
            if (engine.getContext().hasOutput(node->getId())) {
                auto outputs = engine.getContext().getOutputs(node->getId());
                for (auto& [port, value] : outputs) {
                    if (value.type() == typeid(double)) {
                        std::cout << "  Node " << node->getName() 
                                  << " port " << port << " = " 
                                  << std::any_cast<double>(value) << std::endl;
                    }
                }
            }
        }
    });
    
    // 执行（会在断点处暂停）
    std::cout << "Running with breakpoint..." << std::endl;
    engine.run(graph);
    
    // 继续执行
    std::cout << "Resuming..." << std::endl;
    engine.resume(graph);
    
    std::cout << std::endl;
}

// 示例4: 执行到指定节点
void example_run_until() {
    std::cout << "=== Example 4: Run Until Node ===" << std::endl;
    
    Graph graph;
    
    auto num1 = graph.createNode<NumberNode>("Number1", 100.0);
    auto num2 = graph.createNode<NumberNode>("Number2", 50.0);
    auto add = graph.createNode<AddNode>("Add");
    auto mul = graph.createNode<MultiplyNode>("Multiply");
    auto print = graph.createNode<PrintNode>("Print");
    
    graph.connect(num1, 0, add, 0);
    graph.connect(num2, 0, add, 1);
    graph.connect(add, 0, mul, 0);
    graph.connect(num2, 0, mul, 1);
    graph.connect(mul, 0, print, 0);
    
    FlowEngine engine;
    
    // 只执行到 Add 节点
    engine.runUntil(graph, add->getId());
    
    // 查看 Add 节点的输出
    auto addResult = engine.getContext().getOutputAs<double>(add->getId(), 0);
    std::cout << "Add result (stopped here): " << addResult << std::endl;
    
    // 查看执行时间
    std::cout << "Execution records:" << std::endl;
    for (auto& record : engine.getContext().getRecords()) {
        std::cout << "  " << record.nodeName << ": " 
                  << record.executionTime.count() << " us" << std::endl;
    }
    
    std::cout << std::endl;
}

// 示例5: 单独执行节点（调试用）
void example_standalone() {
    std::cout << "=== Example 5: Standalone Node Execution ===" << std::endl;
    
    // 创建一个 Add 节点
    auto add = std::make_shared<AddNode>(1, "TestAdd");
    
    // 手动提供输入进行测试
    std::map<int, std::any> mockInputs;
    mockInputs[0] = 7.0;
    mockInputs[1] = 8.0;
    
    FlowEngine engine;
    auto result = engine.executeNodeStandalone(add, mockInputs);
    
    if (result.success) {
        double output = std::any_cast<double>(result.outputs.at(0));
        std::cout << "7 + 8 = " << output << std::endl;
    } else {
        std::cout << "Error: " << result.errorMessage << std::endl;
    }
    
    std::cout << std::endl;
}

// 主函数
int main() {
    example_basic();
    example_if_branch();
    example_debug();
    example_run_until();
    example_standalone();
    
    std::cout << "All examples completed!" << std::endl;
    return 0;
}
