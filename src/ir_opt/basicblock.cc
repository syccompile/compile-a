#include "basicblock.h"

bool operator==(const IR::Addr &lhs, const IR::Addr &rhs) { // 不适用于比较name字段
  return lhs.kind == rhs.kind && lhs.val == rhs.val;
}

bool operator<(const IR::Addr &lhs, const IR::Addr &rhs) {
  if (lhs.kind != rhs.kind) return lhs.kind < rhs.kind;
  return lhs.val < rhs.val;
}

std::ostream &operator<<(std::ostream &os, IR::Addr a) {
  a.internal_print();
  return os;
}

bool is_mov_op(IR::Op op) {
  return op >= IR::Op::MOV && op <= IR::Op::MOVNE;
}

bool is_algo_op(IR::Op op) {
  if (op == IR::Op::NOT) return false;  // TODO: 会用到吗
  return op >= IR::Op::ADD && op <= IR::Op::XOR;
}

bool is_swappable_op(IR::Op op) {
  return op == IR::Op::ADD || op == IR::Op::MUL ||
      op == IR::Op::AND || op == IR::Op::OR || op == IR::Op::XOR;
}

bool is_jmp_op(IR::Op op) {
  return op >= IR::Op::JMP && op <= IR::Op::JNE;
}

inline bool is_var_or_param(const IR::Addr::Ptr &a) {
  if (a == nullptr) return false;
  return a->kind == IR::Addr::Kind::VAR || a->kind == IR::Addr::Kind::PARAM;
}

inline bool is_imm(const IR::Addr::Ptr &a) {
  return a->kind == IR::Addr::Kind::IMM;
}

void testExp() {
  auto exp1 = Exp(IR::Op::SUB, make_var(1), make_imm(9));
  auto exp2 = Exp(IR::Op::SUB, make_var(1), make_imm(9));
  assert(exp1 == exp2);
  assert(!(exp1 < exp2));
  assert(!(exp2 < exp1));
}

void testIRADDR() {
  auto p0 = make_param(0);
  auto v0 = make_var(0);
  auto v1 = make_var(1);
  assert(!(*p0 < *v0));
  assert(!(*p0 < *v1));
  assert(*v0 < *v1);
  std::list<IR::Addr> li;
  li.emplace_back(IR::Addr::PARAM, 0);
  li.emplace_back(IR::Addr::VAR, 1);
  li.sort();
  assert(li.front().kind == IR::Addr::VAR);
  assert(li.back().kind == IR::Addr::PARAM);
}

std::ostream &operator<<(std::ostream &os, const Exp &exp) {
  exp.a0_->internal_print();
  switch (exp.op_) {
    case IR::Op::ADD: os << "+";
      break;
    case IR::Op::SUB: os << "-";
      break;
    case IR::Op::MUL: os << "*";
      break;
    case IR::Op::DIV: os << "/";
      break;
    case IR::Op::MOD: os << "%";
      break;
    default: os << "=";
  }
  exp.a1_->internal_print();
  return os;
}
bool Exp::operator<(const Exp &exp) const {
  if (op_ != exp.op_) return op_ < exp.op_;
  if (is_swappable_op(op_)) { // 可交换
    if (*a0_ < *a1_) {
      if (*exp.a0_ < *exp.a1_) return *a0_ < *exp.a0_ || *a1_ < *exp.a1_;
      else return *a0_ < *exp.a1_ || *a1_ < *exp.a1_;
    } else {
      if (*exp.a0_ < *exp.a1_) return *a1_ < *exp.a0_ || *a0_ < *exp.a1_;
      else return *a1_ < *exp.a1_ || *a0_ < *exp.a1_;
    }
  } else {
    return *a0_ < *exp.a0_ || *a1_ < *exp.a1_;
  }
}
bool Exp::be_used_by(const IR::Ptr &ir) const {
  if (is_algo_op(ir->op_)) {  // 仅处理算术指令
    if (is_swappable_op(op_)) { // 可交换
      return op_ == ir->op_ &&
          ((*a0_ == *ir->a1 && *a1_ == *ir->a2) || (*a0_ == *ir->a2 && *a1_ == *ir->a1));
    } else {
      return ir->op_ == op_ && *ir->a1 == *a0_ && *ir->a2 == *a1_;
    }
  }
  return false;
}
bool Exp::related_to(const IR_Addr::Ptr &a) const {
  if (is_algo_op(op_)) {
    return (*a0_ == *a) || (*a1_ == *a);
  } else {  // MOV
    return *a1_ == *a || *a0_ == *a;
  }
}
bool Exp::operator==(const Exp &rhs) const {
  if (is_swappable_op(op_)) { // 可交换
    return op_ == rhs.op_ &&
        ((*a0_ == *rhs.a0_ && *a1_ == *rhs.a1_) || (*a0_ == *rhs.a1_ && *a1_ == *rhs.a0_));
  } else {
    return op_ == rhs.op_ && *a0_ == *rhs.a0_ && *a1_ == *rhs.a1_;
  }
}
bool Exp::copy_be_used_by(const IR_Addr::Ptr &a) const {
  if (op_ == IR::Op::MOV) {
    return *a0_ == *a;
  }
  return false;
}
bool Exp::copy_related_to(const IR_Addr::Ptr &a) const {
  if (op_ == IR::Op::MOV) {
    return *a0_ == *a || *a1_ == *a;
  }
  return false;
}

std::list<std::string> BasicBlock::translate_to_arm() {
  return std::list<std::string>();
}
void BasicBlock::debug() {
  for (const auto &ir : ir_list_) {
    ir->internal_print();
  }
}
void BasicBlock::calc_egen_ekill(const std::list<Exp> &all_exp_list) {
  egen_.clear();
  ekill_.clear();

  auto delete_egen_exp = [&](const IR::Addr::Ptr &a) { // 删除和a相关的表达式
    for (auto iter = egen_.begin(); iter != egen_.end();) {
      auto cur_exp = *iter;
      if (cur_exp.related_to(a)) {
        iter = egen_.erase(iter);
      } else {
        ++iter;
      }
    }
  };
  auto delete_ekill_exp = [&](const Exp &exp) {
    for (auto iter = ekill_.begin(); iter != ekill_.end();) {
      if (*iter == exp) {
        iter = ekill_.erase(iter);
      } else {
        ++iter;
      }
    }
  };
  auto add_ekill_exp = [&](const IR::Addr::Ptr &a) {
    for (const auto &exp: all_exp_list) {
      if (exp.related_to(a)) {
        if (std::find(ekill_.begin(), ekill_.end(), exp) == ekill_.end()) {
          ekill_.push_back(exp);
        }
      }
    }
  };
  for (const auto &ir: ir_list_) {
    if (is_algo_op(ir->op_)) { // 算术表达式
      auto new_exp = make_algo_exp(ir);
      egen_.push_back(new_exp);
      delete_egen_exp(ir->a0);
      delete_ekill_exp(new_exp);
      add_ekill_exp(ir->a0);
    } else if (is_mov_op(ir->op_)) {  // 赋值语句
      if (ir->a1->kind == ir->a0->kind && ir->a1->val == ir->a0->val) { // 自赋值
        continue;
      }
      if (ir->op_ == IR::Op::MOV) { // 只有无条件赋值语句才将表达式加入
        delete_egen_exp(ir->a0);
        auto new_exp = make_mov_exp(ir);
        egen_.push_back(new_exp);
        delete_ekill_exp(new_exp);
        add_ekill_exp(ir->a0);
      } else {
        delete_egen_exp(ir->a0);
        add_ekill_exp(ir->a0);
      }
    } else if (ir->op_ == IR::Op::LOAD) { // TODO: 完善LOAD的情形
      delete_egen_exp(ir->a0);
      add_ekill_exp(ir->a0);
    }
  }
  egen_.sort();   // 后续的集合运算要求有序
  ekill_.sort();
}

void BasicBlock::calc_use_def() {
  use_.clear();
  def_.clear();
  auto add_to_def = [&](const IR::Addr::Ptr &a) {
    if (a->kind == IR::Addr::Kind::VAR || a->kind == IR::Addr::Kind::PARAM) {
      auto result = std::find(def_.begin(), def_.end(), *a);
      if (result == def_.end()) {
        def_.push_back(*a);
      }
      use_.remove(*a);
    }
  };
  auto add_to_use = [&](const IR::Addr::Ptr &a) {
    if (a->kind == IR::Addr::Kind::VAR || a->kind == IR::Addr::Kind::PARAM) {
      auto result = std::find(use_.begin(), use_.end(), *a);
      if (result == use_.end()) {
        use_.push_back(*a);
      }
      def_.remove(*a);
    }
  };
  for (auto iter = ir_list_.rbegin(); iter != ir_list_.rend(); ++iter) {
    auto cur_ir = *iter;
    if (is_algo_op(cur_ir->op_)) {
      add_to_def(cur_ir->a0);
      add_to_use(cur_ir->a1);
      add_to_use(cur_ir->a2);
    } else if (is_mov_op(cur_ir->op_)) {
      add_to_def(cur_ir->a0);
      add_to_use(cur_ir->a1);
    } else if (cur_ir->op_ == IR::Op::CMP) {
      add_to_use(cur_ir->a1);
      add_to_use(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::LOAD) {
      add_to_def(cur_ir->a0);
      add_to_use(cur_ir->a1);
      add_to_use(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::STORE) {
      add_to_use(cur_ir->a0);
      add_to_use(cur_ir->a1);
      add_to_use(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::RET) {
      add_to_use(cur_ir->a0);
    } else if (cur_ir->op_ == IR::Op::PARAM) {
      add_to_use(cur_ir->a1);
    }
  }
  use_.sort();
  def_.sort();
}
void BasicBlock::delete_local_common_expression() {
  std::map<Exp, iterator> exp_to_iter;
  std::map<Exp, IR::Addr::Ptr> available_exps;
  auto delete_avail_exp = [&](const IR::Addr::Ptr &a, const iterator &store_iter) {
    auto iter2 = exp_to_iter.begin();
    for (auto iter = available_exps.begin(); iter != available_exps.end();)
      if ((*iter).first.related_to(a)) {
        iter = available_exps.erase(iter);
        iter2 = exp_to_iter.erase(iter2);
      } else {
        ++iter;
        ++iter2;
      }
  };
  for (auto iter = ir_list_.begin(); iter != ir_list_.end(); ++iter) {
    auto cur_ir = *iter;
    if (is_algo_op(cur_ir->op_)) {
      auto exp = make_algo_exp(cur_ir);
      auto result = available_exps.find(exp);
      if (result != available_exps.end()) {
        cur_ir->op_ = IR::Op::MOV;
        if (result->second == nullptr) {
          auto exp_iter = exp_to_iter[exp];
          auto exp_ir = *exp_iter;
          auto tmp_ir = make_tmp_assign_exp_ir(exp);
          ir_list_.insert(exp_iter, tmp_ir);
          exp_ir->op_ = IR::Op::MOV;
          exp_ir->a1 = tmp_ir->a0;
          exp_ir->a2 = nullptr;
          result->second = tmp_ir->a0;
        }
        cur_ir->a1 = result->second;
        cur_ir->a2 = nullptr;
      } else {
        available_exps[exp] = nullptr; // add avail_exp
        exp_to_iter[exp] = iter;
      }
      delete_avail_exp(cur_ir->a0, iter);
    } else if (is_mov_op(cur_ir->op_)) {
      delete_avail_exp(cur_ir->a0, iter);
    }
  }
}
void BasicBlock::constant_folding() {
  for (auto &ir: ir_list_) {
    if (is_algo_op(ir->op_) && ir->a1->kind == IR::Addr::Kind::IMM && ir->a2->kind == IR::Addr::Kind::IMM) {
      switch (ir->op_) {
        case IR::Op::ADD: ir->a1 = make_imm(ir->a1->val + ir->a2->val);
          break;
        case IR::Op::SUB: ir->a1 = make_imm(ir->a1->val - ir->a2->val);
          break;
        case IR::Op::MUL: ir->a1 = make_imm(ir->a1->val * ir->a2->val);
          break;
        case IR::Op::DIV: ir->a1 = make_imm(ir->a1->val / ir->a2->val);
          break;
        case IR::Op::MOD: ir->a1 = make_imm(ir->a1->val % ir->a2->val);
          break;
        default: break; // unreachable!
      }
      ir->op_ = IR::Op::MOV;
      ir->a2 = nullptr;
    }
  }
}
void BasicBlock::algebraic_simplification() {
  for (auto &ir: ir_list_) {
    if (ir->op_ == IR::Op::ADD) {
      if (ir->a1->kind == IR::Addr::Kind::IMM && ir->a1->val == 0) {  // 0+
        ir->op_ = IR::Op::MOV;
        ir->a1 = ir->a2;
        ir->a2 = nullptr;
      } else if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 0) { // +0
        ir->op_ = IR::Op::MOV;
        ir->a2 = nullptr;
      }
    } else if (ir->op_ == IR::Op::SUB) {
      if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 0) { // -0
        ir->op_ = IR::Op::MOV;
        ir->a2 = nullptr;
      }
    } else if (ir->op_ == IR::Op::MUL) {
      if (ir->a1->kind == IR::Addr::Kind::IMM && ir->a1->val == 0) {  // 0*
        ir->op_ = IR::Op::MOV;  // MOV a0 0
        ir->a2 = nullptr;
      } else if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 0) { // *0
        ir->op_ = IR::Op::MOV;  // MOV a0 0
        ir->a1 = ir->a2;
        ir->a2 = nullptr;
      } else if (ir->a1->kind == IR::Addr::Kind::IMM && ir->a1->val == 1) {  // 1*
        ir->op_ = IR::Op::MOV;  // MOV a0 a2
        ir->a1 = ir->a2;
        ir->a2 = nullptr;
      } else if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 1) { // *1
        ir->op_ = IR::Op::MOV;  // MOV a0 a1
        ir->a2 = nullptr;
      }
    } else if (ir->op_ == IR::Op::DIV) {
      if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 1) { // /1
        ir->op_ = IR::Op::MOV;  // MOV a0 a1
        ir->a2 = nullptr;
      }
    } else if (ir->op_ == IR::Op::MOD) {
      // do nothing, DELETE
    } else if (ir->op_ == IR::Op::AND) {
      if (ir->a1->kind == IR::Addr::Kind::IMM && ir->a1->val == 0) {  // 0 &
        ir->op_ = IR::Op::MOV;  // MOV a0 0
        ir->a2 = nullptr;
      } else if (ir->a2->kind == IR::Addr::Kind::IMM && ir->a2->val == 0) { // & 0
        ir->op_ = IR::Op::MOV;  // MOV a0 0
        ir->a1 = ir->a2;
        ir->a2 = nullptr;
      }
    } else if (ir->op_ == IR::Op::OR) {
      // do nothing, DELETE
    } else if (ir->op_ == IR::Op::NOT) {
      // do nothing, DELETE
    } else if (ir->op_ == IR::Op::XOR) {
      // do nothing, DELETE
    }
  }
}
void BasicBlock::local_copy_propagation(std::set<Exp> &available_copy_exps) {
//  std::set<Exp> available_copy_exps;
  auto copy_value = [&](IR::Addr::Ptr &a) {
    if (is_var_or_param(a)) {
      auto iter = available_copy_exps.begin();
      for (; iter != available_copy_exps.end(); ++iter) {
        if ((*iter).copy_be_used_by(a)) {
          a = (*iter).a1_;
        }
      }
    } // ignore else
  };
  auto remove_exp = [&](const IR::Addr::Ptr &a) {
    assert(is_var_or_param(a));
    auto iter = available_copy_exps.begin();
    for (; iter != available_copy_exps.end();) {
      if ((*iter).copy_related_to(a)) {
        iter = available_copy_exps.erase(iter);
      } else {
        ++iter;
      }
    }
  };
  for (auto &ir: ir_list_) {  // TODO: 未处理PARAM,RET,ALLOC_IN_STACK
    if (is_mov_op(ir->op_)) { // 赋值指令
      copy_value(ir->a1);
      remove_exp(ir->a0);
      available_copy_exps.insert(make_mov_exp(ir));
    } else if (is_algo_op(ir->op_)) { // 算术指令
      copy_value(ir->a1);
      copy_value(ir->a2);
      remove_exp(ir->a0);
    } else if (ir->op_ == IR::Op::CMP) {  // 比较指令
      copy_value(ir->a1);
      copy_value(ir->a2);
    } else if (ir->op_ != IR::Op::STORE) {  // 存储
      copy_value(ir->a0);
      copy_value(ir->a1);
      copy_value(ir->a2);
    } else if (ir->op_ != IR::Op::LOAD) { // 取数
      copy_value(ir->a1);
      copy_value(ir->a2);
      remove_exp(ir->a0);
    }
  }
}
void BasicBlock::remove_dead_code() {
  auto live_variables = live_variable_OUT_;
  auto is_live = [&](const IR::Addr::Ptr &a) {
    assert(is_var_or_param(a));
    if (std::find(live_variables.begin(), live_variables.end(), *a) ==
        live_variables.end()) { // 没找到
      return false;
    }
    return true;
  };
  auto add_live = [&](const IR::Addr::Ptr &a) {
    if (a == nullptr) return;
    if (!is_var_or_param(a)) return;
    if (!is_live(a)) {
      live_variables.push_back(*a);
    }
  };
  for (auto iter = std::prev(ir_list_.end()); true; --iter) {
    auto cur_ir = *iter;
    // bool removed = false;
    if (is_mov_op(cur_ir->op_)) { // 赋值操作
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
      } else {  // dead code
        iter = ir_list_.erase(iter);
      }
    } else if (is_algo_op(cur_ir->op_)) { // 算术操作
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
        add_live(cur_ir->a2);
      } else {  // dead code
        iter = ir_list_.erase(iter);
      }
    } else if (cur_ir->op_ == IR::Op::CMP) {  // 比较操作
      add_live(cur_ir->a1);
      add_live(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::LOAD) {
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
        add_live(cur_ir->a2);
      } else {
        iter = ir_list_.erase(iter);
      }
    } else if (cur_ir->op_ == IR::Op::STORE) {
      add_live(cur_ir->a0);
      add_live(cur_ir->a1);
      add_live(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::PARAM) {
      add_live(cur_ir->a1);
    } else if (cur_ir->op_ == IR::Op::RET) {
      add_live(cur_ir->a0);
    }
    if (iter == ir_list_.begin()) break;
  }
}
void BasicBlock::ir_specify_optimization() {
  /*
   * 将类似：ADD reg1, X, X
   *        MOV reg2, reg1
   * 的代码转换成： ADD reg2, X, X
   * */
  auto live_variables = live_variable_OUT_;
  auto is_live = [&](const IR::Addr::Ptr &a) {
    assert(is_var_or_param(a));
    if (std::find(live_variables.begin(), live_variables.end(), *a) ==
        live_variables.end()) { // 没找到
      return false;
    }
    return true;
  };
  auto add_live = [&](const IR::Addr::Ptr &a) {
    if (a == nullptr) return;
    if (!is_var_or_param(a)) return;
    if (!is_live(a)) {
      live_variables.push_back(*a);
    }
  };
  for (auto iter = std::prev(ir_list_.end()); true; --iter) {
    auto cur_ir = *iter;

    if (iter == ir_list_.begin()) break;
    auto pre_ir = *(std::prev(iter));
    if (cur_ir->op_ != IR::Op::MOV) goto update_live;
    if (!is_algo_op(pre_ir->op_)) goto update_live; // TODO: 对于LOAD指令也许也可以使用
    if (!(*(cur_ir->a1) == *(pre_ir->a0))) goto update_live;;
    if (is_live(cur_ir->a1)) goto update_live;
    pre_ir->a0 = cur_ir->a0;
    iter = ir_list_.erase(iter);  // 删除MOV指令
    --iter;
    if (iter == ir_list_.begin()) break;

update_live:
    if (is_mov_op(cur_ir->op_)) { // 赋值操作
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
      } // ignore else
    } else if (is_algo_op(cur_ir->op_)) { // 算术操作
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
        add_live(cur_ir->a2);
      } // ignore else
    } else if (cur_ir->op_ == IR::Op::CMP) {  // 比较操作
      add_live(cur_ir->a1);
      add_live(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::LOAD) {
      if (is_live(cur_ir->a0)) {
        add_live(cur_ir->a1);
        add_live(cur_ir->a2);
      } // ignore else
    } else if (cur_ir->op_ == IR::Op::STORE) {
      add_live(cur_ir->a0);
      add_live(cur_ir->a1);
      add_live(cur_ir->a2);
    } else if (cur_ir->op_ == IR::Op::PARAM) {
      add_live(cur_ir->a1);
    } else if (cur_ir->op_ == IR::Op::RET) {
      add_live(cur_ir->a0);
    }
  }
}

void BasicBlock::if_simplify() {
  auto mov_successfully = [](IR::Op op, int left, int right) -> bool {
    assert(op != IR::Op::MOV && op != IR::Op::MVN);  // 不能是MOV
    if ((op == IR::Op::MOVLE && left <= right) ||
        (op == IR::Op::MOVLT && left < right) ||
        (op == IR::Op::MOVGE && left >= right) ||
        (op == IR::Op::MOVGT && left > right) ||
        (op == IR::Op::MOVEQ && left == right) ||
        (op == IR::Op::MOVNE && left != right)) {
      return true;
    }
    return false;
  };
  auto jmp_successfully = [](IR::Op op, int left, int right) -> bool {
    assert(op != IR::Op::JMP);  // 不能是JMP
    if ((op == IR::Op::JLE && left <= right) ||
        (op == IR::Op::JLT && left < right) ||
        (op == IR::Op::JGE && left >= right) ||
        (op == IR::Op::JGT && left > right) ||
        (op == IR::Op::JE && left == right) ||
        (op == IR::Op::JNE && left != right)) {
      return true;
    }
    return false;
  };
  for (auto iter = ir_list_.begin(); iter != ir_list_.end();) {
    auto cur_ir = *iter;
    if (cur_ir->op_ == IR::Op::CMP && is_imm(cur_ir->a1) && is_imm(cur_ir->a2)) {
      auto left = cur_ir->a1->val, right = cur_ir->a2->val;
      auto next_iter = std::next(iter);
      auto next_ir = *next_iter;
      ir_list_.erase(iter); // 删除CMP指令
      IR::Op next_ir_op = next_ir->op_;
      if (is_mov_op(next_ir_op)) {  // 可能修改多条指令
        assert(next_ir_op != IR::Op::MOV && next_ir_op != IR::Op::MVN);  // DELETE: 理论上不会是MOV
        while (true) {
          if (mov_successfully(next_ir_op, left, right)) {
            next_ir->op_ = IR::Op::MOV; // 变为无条件赋值
            ++next_iter;  // 跳转到下一条指令
          } else {  // 直接删除
            next_iter = ir_list_.erase(next_iter);
          }
          if (next_iter == ir_list_.end()) break;
          next_ir = *next_iter;
          next_ir_op = next_ir->op_;
          if (!is_mov_op(next_ir_op) || next_ir_op == IR::Op::MOV || next_ir_op == IR::Op::MVN) {
            break;
          }
        }
      } else if (is_jmp_op(next_ir_op)) { // 最多修改一条指令
        if (jmp_successfully(next_ir_op, left, right)) {
          next_ir->op_ = IR::Op::JMP;  // 变成无条件跳转
          ++next_iter;
        } else {
          next_iter = ir_list_.erase(next_iter);  // 直接删除
        }
      } else {
        assert(false);  // DELETE: 理论上不会到达这个分支
      }
      iter = next_iter;
    } else {
      ++iter;
    }
  }
}
