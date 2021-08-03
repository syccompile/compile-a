#include "color_graph.h"
#include <stack>
#include <map>

namespace color_graph {

namespace { // helper
// 为一个节点着色
void _colorize_node(color_node::ptr p, color_allocate& alloc) {
  if (p->is_colored()) {
    return ;
  }
  color_node::nodes neighbors = p->get_colored_neighbors();
  // 如果已着色的邻居数量比已分配颜色的数量更多，
  // 只能分配一个新的颜色
  if (neighbors.size() >= alloc.alloc_num()) {
    p->colorize(alloc.get_new_color());
    return ;
  }
  // 如果已着色的邻居数量比已分配的颜色数量少，
  // 必然存在一个已分配过的颜色与邻居的颜色都不冲突
  // 尝试使用一个已分配过的颜色为节点分配
	@@ -34,6 +40,7 @@ void _colorize_node(color_node::ptr p, color_allocate& alloc) {
    if (no_neighbor_use) {
      // 找到一个没有邻居使用过的颜色
      p->colorize(i);
      break;
    }
  }
	@@ -53,40 +60,25 @@ void color_allocate::reclaim_color() {
  allocated_num--;
}

void colorize_nodes(color_node::nodes nodes) {
  color_allocate alloc;
  colorize_nodes_allocate(nodes, alloc);
}

void colorize_nodes_allocate(color_node::nodes nodes, color_allocate alloc) {
  for (color_node::ptr p : nodes) {
    _colorize_node(p, alloc);
  }
}

void colorize_num_allocate(color_node::nodes nodes, int color_num, color_allocate alloc) {
  for (color_node::ptr p : nodes) {
    _colorize_node(p, alloc);
    // 变量实际溢出
    if (alloc.alloc_num() > color_num) {
      alloc.reclaim_color();
      p->colorize(spill_color());
    }
  }
}

void colorize_num(color_node::nodes nodes, int color_num) {
  color_allocate alloc;
  colorize_num_allocate(nodes, color_num, alloc);
}

color_node::nodes color_node::get_colored_neighbors() {
  nodes n;
  for (ptr p : get_neighbors()) {
    if (p->is_colored()) {
      n.insert(p);
    }
  }
  return n;
}
} // namespace color_graph
