#include "../ir.h"
#include "flow_graph.h"

#include <assert.h>
#include <tuple>
#include <vector>
#include <list>

namespace  // helper
{
/**
 * 集合相减，返回一个只包含元素a的集合, a属于lhs,且a不属于rhs
 */
template <class T> set<T> set_sub(set<T> lhs, set<T> rhs) {
  set<T> ret;
  for (T t : lhs) {
    if (find(rhs.begin(), rhs.end(), t) == rhs.end()) {
      ret.insert(t);
    }
  }
  return ret;
}

/**
 * 集合合并，返回一个只包含元素a的集合, a属于lhs,或a属于rhs
 * post-condition: 返回集合不包含相同的元素
 */
template <class T> set<T> set_union(set<T> lhs, set<T> rhs) {
  set<T> ret = lhs;
  for (T t : rhs) {
    if (find(lhs.begin(), lhs.end(), t) == lhs.end()) {
      ret.insert(t);
    }
  }
  return ret;
}

IR::Ptr find_label(IR::List list, IR_Addr::Ptr label) {
  for (auto ir : list) {
    if (ir->op_ == IR::Op::LABEL && ir->a0 == label) {
      return ir;
    }
  }
  return nullptr;
}

#define ACCEPT_ADDR_NOT_IMM(set, addr)                                         \
  if (ir->addr->kind != IR_Addr::Kind::IMM) {                                  \
    ir->set.insert(ir->addr);                                                  \
    vars.push_back(ir->addr);                                                  \
  }                                                                            \
  if (ir->addr->kind == IR_Addr::Kind::PARAM) {                                \
    params.push_back(ir->addr);                                                \
  }

tuple<vector<color_node::ptr>, vector<color_node::ptr>>
ir_parse(IR::List ir_list) {
  // 函数中使用的所有变量
  vector<color_node::ptr> vars;
  // 变量中函数的参数
  vector<color_node::ptr> params;
  IR::Ptr prev_ir;
  for (auto ir : ir_list) {
    switch (ir->op_) {
#define OP_CASE(op) case IR::Op::op:
      OP_CASE(ADD)
      OP_CASE(SUB)
      OP_CASE(MUL)
      OP_CASE(DIV)
      OP_CASE(MOD) {
          ACCEPT_ADDR_NOT_IMM(used, a1);
          ACCEPT_ADDR_NOT_IMM(used, a2);
          ACCEPT_ADDR_NOT_IMM(def, a0);
          break;
        }
      OP_CASE(CMP)
      OP_CASE(MOVLE)
      OP_CASE(MOVLT)
      OP_CASE(MOVGE)
      OP_CASE(MOVGT)
      OP_CASE(MOVEQ)
      OP_CASE(MOVNE)
      OP_CASE(MOV) {
          ACCEPT_ADDR_NOT_IMM(used, a1);
          ACCEPT_ADDR_NOT_IMM(def, a0);
          break;
      }
      OP_CASE(JMP)
      OP_CASE(JLE)
      OP_CASE(JLT)
      OP_CASE(JGE)
      OP_CASE(JGT)
      OP_CASE(JE)
      OP_CASE(JNE) {
        auto label_ir = find_label(ir_list, ir->a0);
        ir->succ.push_back(label_ir);
        label_ir->pred.push_back(ir);
        break;
      }
      OP_CASE(STORE) {
        ACCEPT_ADDR_NOT_IMM(used, a0);
        ACCEPT_ADDR_NOT_IMM(used, a1);
        ACCEPT_ADDR_NOT_IMM(used, a2);
        break;
      }
      OP_CASE(LOAD) {
        ACCEPT_ADDR_NOT_IMM(def, a0);
        ACCEPT_ADDR_NOT_IMM(used, a1);
        break;
      }
      OP_CASE(LABEL)
      OP_CASE(FUNCDEF)
      OP_CASE(FUNCEND)
      OP_CASE(PARAM)
      OP_CASE(CALL)
      OP_CASE(RET)
      OP_CASE(VARDEF)
      OP_CASE(DATA)
      OP_CASE(ZERO)
      OP_CASE(VAREND)
      OP_CASE(ALLOC_IN_STACK)
      OP_CASE(NOP) {
        break;
      }
#undef OP_CASE
    }
    if (prev_ir != nullptr) {
      ir->pred.push_back(prev_ir);
      prev_ir->succ.push_back(ir);
    }
    prev_ir = ir;
  }
  return make_tuple(vars, params);
}
#undef ACCEPT_ADDR_NOT_IMM

// 构造冲突图
// 由函数liveness_analyze调用
void make_conflict_graph(vector<varUse::ptr> var_uses, vector<color_node::ptr> nodes) {
  // 1. 对于对变量a的def操作, 为a与出口活跃的变量添加冲突边, 例如 a <- b + c ...
  // 2. 对于传送指令 a <- c ， 为a与出口活跃的变量除了c外的变量添加冲突边
  vector<shared_ptr<var>> vars;
  for(auto node : nodes ){
    // 指针转换必须是有效的
    vars.push_back(dynamic_pointer_cast<var>(node));
  }
  for (shared_ptr<varUse> t : var_uses) {
    // def 最多只能有一个元素
    if (t->def.size() == 0) {
      continue;
    }
    auto def = *(t->def.begin());
    auto ir_ptr = dynamic_pointer_cast<IR>(t);
    assert(ir_ptr);
    if (ir_ptr->op_ == IR::Op::MOV) {
      // 标记传送指令
      t->is_move = true;
    }
    for (auto out : t->out) {
      if (t->is_move && out == *(t->used.begin())) {
        // 对于传送指令
        continue;
      }
      var::link(def, out);
    }
  }
}
} // namespace

// 活跃分析
// pre-condition: ir_list必须是一个函数的完整ir
//      ir_list不能为空
void liveness_analyze(IR::List ir_list) {
  vector<color_node::ptr> vars, params;
  std::tie(vars, params) = ir_parse(ir_list);
  std::vector<varUse::ptr> nodes;
  for (auto ir : ir_list) { nodes.push_back(ir);
  }
  while (true) {
    int immut_time = 0;
    for (varUse::ptr n : nodes) {
      set<shared_ptr<var>> in = n->in, out = n->out;
      // in[n] = use[n] U (out[n] - def[n])
      n->in = set_union(n->used, set_sub(n->out, n->def));

      // out[n] = U in[s] ; s in succ[n]
      n->out.clear();
      for (varUse::ptr succ : n->succ) {
        n->out = set_union(n->out, succ->in);
      }
      // 判断结束条件
      if (n->in.size() == in.size() && n->out.size() == out.size()) {
        immut_time ++;
      }
    }
    // 该次循环中没有结点发生变化则退出
    if (immut_time == nodes.size()) {
      break;
    }
  }

  // 构造冲突图
  make_conflict_graph(nodes, vars);
  // 预先着色函数参数
  color_allocate alloc;
  for (auto param : params) {
    param->colorize(alloc.get_new_color());
  }

  colorize_nodes_allocate(vars, alloc);
}
