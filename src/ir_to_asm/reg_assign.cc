#include "../ir.h"
#include "../reg_allocate/color_graph.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

const int kTransfer_Reg = 4;
const int kMax_Use_Reg  = 7;

std::unordered_map<int, int>
register_assign(IR::List &l) {
  std::unordered_map<int, IR::List::iterator> label_map;
  std::vector<std::pair<int, int> > color_prior;

  auto add_prior = [&color_prior](IR::Addr::Ptr ir_addr, bool in_loop) {
    if (ir_addr->get_color() == color_graph::none_color()) return;
    if (ir_addr->get_color() <  kTransfer_Reg) return;
    if (color_prior.size()<=ir_addr->get_color()) color_prior.resize(ir_addr->get_color()+1);

    int prior = color_prior[ir_addr->get_color()].first;
    if (in_loop) prior += 10;
    else         prior += 1;
    color_prior[ir_addr->get_color()].first = prior;
  };

  for (auto it=l.begin() ; it!=l.end() ; ++it) {
    if ((*it)->a0->kind == IR::Addr::BRANCH_LABEL) {
      label_map[(*it)->a0->val] = it;
    }
    else if ((*it)->is_jmp() && label_map.count((*it)->a0->val)) {
      for (auto jt=label_map[(*it)->a0->val] ; jt!=it ; ++jt) (*jt)->in_inner_loop = true;
    }
  }

  for (auto ir: l) {
    if (ir->a0!=nullptr) add_prior(ir->a0, ir->in_inner_loop);
    if (ir->a1!=nullptr) add_prior(ir->a1, ir->in_inner_loop);
    if (ir->a2!=nullptr) add_prior(ir->a2, ir->in_inner_loop);
  }

  for (int i=0 ; i<color_prior.size() ; i++) {
    color_prior[i].second = i;
  }

  std::sort(color_prior.begin(), color_prior.end(), std::greater<std::pair<int, int> >());

  std::unordered_map<int, int> ret;
  int in_reg = std::max(color_prior.size(), size_t(kMax_Use_Reg));
  for (int i=0 ; i<in_reg ; i++) ret[i] = i;
}