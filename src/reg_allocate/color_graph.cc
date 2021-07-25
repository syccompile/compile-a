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
  // FIX: 更优的着色算法
  for (color i = none_color() + 1; i <= alloc.alloc_num(); ++i) {
    // 遍历所有已分配过的颜色
    bool no_neighbor_use = true;
    for (color_node::ptr n : neighbors) {
      if (n->get_color() == i) {
        // 如果该颜色已被邻居使用，该颜色不可用
        no_neighbor_use = false;
        break;
      }
    }
    if (no_neighbor_use) {
      // 找到一个没有邻居使用过的颜色
      p->colorize(i);
      break;
    }
  }
}

// Briggs MOV节点合并判断
// 如果合并后节点的高度数(>=K)邻接点个数少于K，则它们可以合并
bool _briggs_mov(color_node::ptr fir, color_node::ptr sec) {
  int high_deg = 0;

  for (auto &ptr: fir->get_neighbors()) {
    if (ptr->get_neighbors().size()>=6) high_deg++;
    if (high_deg >= 6) return false;
  }

  for (auto &ptr: sec->get_neighbors()) {
    if (ptr->get_neighbors().size()>=6) high_deg++;
    if (high_deg >= 6) return false;
  }

  return high_deg < 6;
}

} // helper

color none_color() { return 0; }

color spill_color() { return -1; }

color color_allocate::get_new_color() { return ++allocated_num; }

int color_allocate::alloc_num() { return allocated_num; }

void color_allocate::reclaim_color() {
  allocated_num--;
}

void process_mov(std::pair<color_node::ptr, color_node::ptr> pnn, color_allocate & alloc) {
  auto node1 = pnn.first,
       node2 = pnn.second;

  bool can_mov = _briggs_mov(node1, node2);

  if (!can_mov) return;

  if (node1->is_colored()) {
    if (node2->is_colored()) return;
    else node2->colorize(node1->get_color());
  }
  else {
    if (node2->is_colored()) node1->colorize(node2->get_color());
    else {
      _colorize_node(node1, alloc);
      node2->colorize(node1->get_color());
    }
  }
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
