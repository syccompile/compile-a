#pragma once

#include <vector>
#include <set>
#include <memory>
#include <utility>
#include <map>

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
  color_node(): col(none_color()) {}
  // Check whether node has been colorized
  bool is_colored() { return col != none_color(); }
  // Colorize a node with color c
  void colorize(color c) { col = c; }
  // Get color of a node
  color get_color() { return col; }
  // Get node's neighbors who was colored
  nodes get_colored_neighbors(std::map<int, int> num_color_list);
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


// 使用显式的颜色分配器
// 对于预着色节点可以
// 使用颜色分配器为预着色
// 节点分配颜色后再将该分配器传入
void colorize_nodes_allocate(color_node::nodes nodes, color_allocate alloc, std::map<int, int> &map);

}
