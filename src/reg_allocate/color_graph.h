#pragma once

#include <vector>
#include <set>
#include <memory>

namespace color_graph{

using color = int;
// 表示没有颜色
color none_color();
// 表示变量实际溢出的颜色
color spill_color();

class color_node {
public:
  using ptr = std::shared_ptr<color_node>;
  using nodes = std::set<ptr>;
  color_node() {}
  // Check whether node has been colorized
  bool is_colored() { return col != none_color(); }
  // Colorize a node with color c
  void colorize(color c) { col = c; }
  // Get color of a node
  color get_color() { return col; }
  // Get node's neighbors who was colored
  nodes get_colored_neighbors();
  // Get all the node's neighbors
  virtual nodes get_neighbors() = 0;
  // Check whether a node is neighbored
  virtual bool is_neighbored(ptr) = 0;

  virtual ~color_node() {}
private : 
  // The color of a node
  color col;
};

class color_allocate {
public:
  // 返回一个新的color
  color get_new_color();

  // 已经分配的颜色
  int alloc_num();

  // 回收最后一个分配的颜色
  void reclaim_color();

  // 目前已经分配的颜色数目
  color allocated_num = none_color();
};


// 为一系列相邻的节点着色
// pre-condition: 所有节点相邻的节点都在集合中
void colorize_nodes(color_node::nodes nodes);

// 尝试使用color_num种颜色为图着色,
// 可能会不够，所以有些节点会是spill
// color 表示溢出
void colorize_num(color_node::nodes nodes, int color_num);

// 使用显式的颜色分配器
// 对于预着色节点可以
// 使用颜色分配器为预着色
// 节点分配颜色后再将该分配器
// 传入
void colorize_nodes_allocate(color_node::nodes nodes,
                             color_allocate alloc);

// 使用显式分配器分配器
// 如果已经为预着色节点
// 分配颜色 已分配颜色数
// 也计入alloc_num
void colorize_num_allocate(color_node::nodes nodes, int color_num, color_allocate alloc);
}
