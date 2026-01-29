#pragma once
#include "./Node.h"

class Edge {

private:
	int id_;
	Node* src_;       // 源节点, 边的数据就来源于源节点的计算。源节点是边的生产者
	Node* dst_;       // 目标节点，边的数据提供给目标节点进行计算。目标节点是边的消费者
	int src_output_;  // 表示当前边为源节点的第src_output_条边。源节点可能会有多条输出边  
	int dst_input_;   // 表示当前边为目标节点的第dst_input_条边。目标节点可能会有多条输入边。
};