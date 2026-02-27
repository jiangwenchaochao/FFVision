// FlowEngine 使用示例

#include "Flow.h"
#include <iostream>

using namespace flow;

// 示例1: 简单的数学计算
void example_basic() {
    std::cout << "=== Example " << std::endl;
    
    Graph graph;
    
    auto source = graph.createNode<ImageSourceNode>("source1","D:\\fake_camera\\0\\ng_3.png");
    auto cropImg = graph.createNode<ImageCropNode>("crop1",100,100);
    
    auto saveImg = graph.createNode<ImageSaveNode>("save1","D://cropImg","11.png");
    
    // 连接
    graph.connect(source, 0, cropImg, 0);  // result -> print
    graph.connect(cropImg, 0, saveImg, 0);  // result -> print
    
    // 执行
    FlowEngine engine;
    engine.run(graph);
    
    std::cout << std::endl;
}

// 主函数
int main() {
    example_basic();
    
    std::cout << "All examples completed!" << std::endl;
    return 0;
}
