#include "function.h"

const std::string normal = "\033[0m";
const std::string black = "\033[0;30m";
const std::string red = "\033[0;31m";
const std::string green = "\033[0;32m";
const std::string yellow = "\033[0;33m";
const std::string blue = "\033[0;34m";
const std::string white = "\033[0;37m";

/* 打印一个map<int, vector<int>> */
void PRINT_MAP(const std::map<int, std::list<int>> &m) {
  for (const auto &val : m) {
    std::cout << val.first << ": ";
    for (const auto &a : val.second) {
      std::cout << a << " ";
    }
    std::cout << std::endl;
  }
}

template<typename T>
inline void PRINT_ELEMENTS(const T &coll,
                           const std::string &prompt = "",
                           const std::string &sep = " ",
                           const std::string &endstr = "\n") {
  std::cout << prompt;
  for (const auto &elem: coll) {
    std::cout << elem << sep;
  }
  std::cout << endstr;
}

void PRINT_PRED_SUCC_BLOCKS(const std::list<BasicBlock::Ptr_weak> &l) {
  for (const auto &elem: l) {
    std::cout << (elem.lock())->block_num_ << " ";
  }
  std::cout << std::endl;
}

void PRINT_LOOP(const std::vector<BasicBlock::Ptr> &l) {
  for (const auto &elem: l) {
    std::cout << elem->block_num_ << " ";
  }
}

void Function::_divide_basic_block(list<IR::Ptr> &ir_list) {
  basic_block_vector_.clear();
  auto tmp_basic_block = make_empty_basic_block();  // 第一条指令是首指令
  while (!(ir_list.empty())) {
    auto front = ir_list.front();
    if (front->op_ == IR::LABEL) {
      if (!(tmp_basic_block->ir_list_.empty())) {
        basic_block_vector_.push_back(tmp_basic_block);
        tmp_basic_block = make_empty_basic_block(); // 跳转指令的目标指令是首指令
      }
      tmp_basic_block->ir_list_.push_back(front);
    } else if ((front->op_ >= IR::JMP && front->op_ <= IR::JNE) ||
        (front->op_ == IR::RET)) {
      tmp_basic_block->ir_list_.push_back(front);
      basic_block_vector_.push_back(tmp_basic_block);
      tmp_basic_block = make_empty_basic_block(); // 跳转指令的后一条指令是首指令
    } else {
      tmp_basic_block->ir_list_.push_back(front);
    }
    ir_list.pop_front();
  }
  if (!(tmp_basic_block->ir_list_.empty())) {
    basic_block_vector_.push_back(tmp_basic_block);
  }
}

void Function::_link_basic_block() {
  // 计算前驱节点和后继节点
  auto find_label = [&](int label_num) {
    for (const auto &basic_block : basic_block_vector_) {
      auto first_ir = basic_block->ir_list_.front();
      if (first_ir->op_ == IR::LABEL && first_ir->a0->val == label_num) {
        return basic_block;
      }
    }
    throw string("could not find label ") + to_string(label_num);
  };

  for (auto iter = basic_block_vector_.begin(); iter != basic_block_vector_.end(); ++iter) {
    auto basic_block = *iter;
    auto last_ir = basic_block->ir_list_.back();
    try {
      if (last_ir->op_ == IR::JMP) {  // 无条件跳转
        auto target_block = find_label(last_ir->a0->val);
        basic_block->successor_list_.push_back(target_block);
        target_block->predecessor_list_.push_back(basic_block);
      } else if (last_ir->op_ == IR::RET) { // 无条件跳转
        if (next(iter) != basic_block_vector_.end()) {
          auto next_block = *next(iter);
          basic_block->successor_list_.push_back(next_block);
          next_block->predecessor_list_.push_back(basic_block);
        }
      } else if (last_ir->op_ >= IR::JLE && last_ir->op_ <= IR::JNE) {  // 条件跳转
        if (next(iter) != basic_block_vector_.end()) {
          auto next_block = *next(iter);
          auto target_block = find_label(last_ir->a0->val);
          if (next_block->block_num_ != target_block->block_num_) {
            basic_block->successor_list_.push_back(next_block);
            next_block->predecessor_list_.push_back(basic_block);
          } // 两个目标相同则只建立前驱-后继关系一次，不确保该情况会出现
          basic_block->successor_list_.push_back(target_block);
          target_block->predecessor_list_.push_back(basic_block);
        } else {
          auto target_block = find_label(last_ir->a0->val);
          basic_block->successor_list_.push_back(target_block);
          target_block->predecessor_list_.push_back(basic_block);
        }
      } else {
        if (next(iter) != basic_block_vector_.end()) {
          auto next_block = *next(iter);
          basic_block->successor_list_.push_back(next_block);
          next_block->predecessor_list_.push_back(basic_block);
        }
      }
    } catch (const string &e) {
      cout << e << endl;
      exit(EXIT_FAILURE);
    }
  }
}

Function::Function(list<IR::Ptr> &ir_list) {
  func_name_ = ir_list.front()->a0->name;
  arg_num_ = ir_list.front()->a1->val;
  ir_list.pop_front();  // pop FUNCDEF

  _divide_basic_block(ir_list);
  _build_lineno_ir_map();
  _link_basic_block();
}

list<string> Function::translate_to_arm() {
  return list<string>();
}
void Function::debug() {
//  testIRADDR();
  cout << green << func_name_ << ": " << normal << endl;
  cout << "------------------------" << endl;
//  for (int i = 0; i < 2; ++i) {
//    constant_folding();
//    algebraic_simplification();
//    delete_local_common_expression();
//    delete_global_common_expression();
//    local_copy_propagation();
//    global_copy_propagation();
////    ir_specify_optimization();
//    if_simplify();
//    staighten();
//    delete_unreachable_code();
//    loop_invariant_code_motion();
//    remove_dead_code();
//  }
  reach_define_analysis();
  available_expression_analysis();
  live_variable_analysis();
  for (const auto &basic_block : basic_block_vector_) {
    cout << red << "block " << basic_block->block_num_ << ":" << normal << endl;
    basic_block->debug();
    cout << blue << "predecessor: " << normal;
    PRINT_PRED_SUCC_BLOCKS(basic_block->predecessor_list_);
    cout << blue << "successor: " << normal;
    PRINT_PRED_SUCC_BLOCKS(basic_block->successor_list_);
    cout << blue << "gen: " << normal;
    PRINT_ELEMENTS(basic_block->gen_);
    cout << blue << "kill: " << normal;
    PRINT_ELEMENTS(basic_block->kill_);
    cout << blue << "reach_define_IN: " << normal;
    PRINT_ELEMENTS(basic_block->reach_define_IN_);
    cout << blue << "reach_define_OUT: " << normal;
    PRINT_ELEMENTS(basic_block->reach_define_OUT_);
    cout << blue << "egen: " << normal;
    PRINT_ELEMENTS(basic_block->egen_);
    cout << blue << "ekill: " << normal;
    PRINT_ELEMENTS(basic_block->ekill_);
    cout << blue << "available_expression_IN_: " << normal;
    PRINT_ELEMENTS(basic_block->available_expression_IN_);
    cout << blue << "available_expression_OUT_: " << normal;
    PRINT_ELEMENTS(basic_block->available_expression_OUT_);
    cout << blue << "use: " << normal;
    PRINT_ELEMENTS(basic_block->use_);
    cout << blue << "def: " << normal;
    PRINT_ELEMENTS(basic_block->def_);
    cout << blue << "live_variable_IN_: " << normal;
    PRINT_ELEMENTS(basic_block->live_variable_IN_);
    cout << blue << "live_variable_OUT_: " << normal;
    PRINT_ELEMENTS(basic_block->live_variable_OUT_);
    cout << blue << "dominate_IN_: " << normal;
    PRINT_ELEMENTS(basic_block->dominate_IN_);
    cout << blue << "dominate_OUT_: " << normal;
    PRINT_ELEMENTS(basic_block->dominate_OUT_);
    cout << endl;
  }
  cout << blue << "loops: " << normal << '\n';
  vector<loop> loops;
  _find_back_edges();
  for (const auto &e : back_edges_) {
    loops.push_back(_get_loop(e));
  }
  for (auto &l : loops) {
    auto loop_invariant_set = _mark_loop_invariant(l);
    cout << "( ";
    PRINT_LOOP(l);
    cout << "): ";
    for (auto[block, iter]: loop_invariant_set) {
      cout << "(" << block->block_num_ << ", " << distance(block->begin(), iter) << ")" << " ";
    }
    cout << '\n';
  }
  cout << '\n';
//  std::cout << blue << "back_edges_: " << normal << std::endl;
//  for (const auto &[first, second] : back_edges_) {
//    std::cout << "(" << first << "->" << second << ") ";
//  }
//  std::cout << '\n';
//  std::cout << "gen_map: " << std::endl;
//  PRINT_MAP(gen_map_);
//  std::cout << "kill_map: " << std::endl;
//  PRINT_MAP(kill_map_);
  cout << "------------------------\n" << endl;
}
void Function::_build_gen_kill_map() {
  gen_kill_help_map_.clear();
  gen_map_.clear();
  kill_map_.clear();
  int lineno = -1;
  for (const auto &basic_block : basic_block_vector_) {
    for (const auto &ir : *basic_block) {
      ++lineno;
      _add_to_gen_kill_help_map(ir, lineno);
    }
  }
  lineno = -1;
  for (const auto &basic_block : basic_block_vector_) {
    for (const auto &ir : *basic_block) {
      ++lineno;
      _add_to_gen_map(ir, lineno);
      _add_to_kill_map(ir, lineno);
    }
  }
}
void Function::_add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::LOAD)) {
    if (ir->a0->kind == IR_Addr::PARAM) {  // 函数参数
      gen_kill_help_map_.insert(decltype(gen_kill_help_map_)::value_type(ir->a0->val, lineno));
    } else {  // 普通变量
      gen_kill_help_map_.insert(decltype(gen_kill_help_map_)::value_type(ir->a0->val + arg_num_, lineno));
    }
  } // ignore else
}
void Function::_add_to_gen_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::LOAD)) {
    gen_map_[lineno].push_back(lineno);
  } // ignore else
}
void Function::_add_to_kill_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::LOAD)) {
    int val = ir->a0->val;
    decltype(gen_kill_help_map_)::iterator val_beg, val_end;
    if (ir->a0->kind == IR_Addr::PARAM) {  // 函数参数
      val_beg = gen_kill_help_map_.lower_bound(val);
      val_end = gen_kill_help_map_.upper_bound(val);
    } else {  // 普通变量
      val_beg = gen_kill_help_map_.lower_bound(val + arg_num_);
      val_end = gen_kill_help_map_.upper_bound(val + arg_num_);
    }
    while (val_beg != val_end) {
      if (val_beg->second != lineno) {
        kill_map_[lineno].push_back(val_beg->second);
      }
      val_beg++;
    }
  } // ignore else
}
void Function::_calc_gen_kill() {
  _build_lineno_ir_map();
  _build_gen_kill_map();
  for (const auto &basic_block : basic_block_vector_) {
    basic_block->gen_.clear();
    basic_block->kill_.clear();
    list<int> gen_list, kill_list, tmp_gen_list, tmp_kill_list, tmp_difference_list;
    for (int cur_lineno = basic_block->last_lineno_; cur_lineno >= basic_block->first_lineno_; --cur_lineno) {
      set_difference(gen_map_[cur_lineno].begin(), gen_map_[cur_lineno].end(),
                     kill_list.begin(), kill_list.end(),
                     back_inserter(tmp_difference_list));
      set_union(gen_list.begin(), gen_list.end(),
                tmp_difference_list.begin(), tmp_difference_list.end(),
                back_inserter(tmp_gen_list));
      set_union(kill_list.begin(), kill_list.end(),
                kill_map_[cur_lineno].begin(), kill_map_[cur_lineno].end(),
                back_inserter(tmp_kill_list));
      gen_list.swap(tmp_gen_list);
      kill_list.swap(tmp_kill_list);
      tmp_gen_list.clear();
      tmp_kill_list.clear();
      tmp_difference_list.clear();
    }
    basic_block->gen_.swap(gen_list);
    basic_block->kill_.swap(kill_list);
  }
}
void Function::_calc_reach_define_IN_OUT() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->reach_define_OUT_.clear();
  }
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_vector_) {
      list<int> tmp_IN, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &pred_block: basic_block->predecessor_list_) {
        set_union(tmp_IN.begin(), tmp_IN.end(),
                  pred_block.lock()->reach_define_OUT_.begin(),
                  pred_block.lock()->reach_define_OUT_.end(),
                  back_inserter(tmp));
        tmp_IN.swap(tmp);
        tmp.clear();
      }
      basic_block->reach_define_IN_.swap(tmp_IN);
      set_difference(basic_block->reach_define_IN_.begin(),
                     basic_block->reach_define_IN_.end(),
                     basic_block->kill_.begin(), basic_block->kill_.end(),
                     back_inserter(tmp_difference_list));
      set_union(basic_block->gen_.begin(), basic_block->gen_.end(),
                tmp_difference_list.begin(), tmp_difference_list.end(),
                back_inserter(tmp_OUT));
      if (basic_block->reach_define_OUT_ != tmp_OUT) {
        change = true;
        basic_block->reach_define_OUT_.swap(tmp_OUT);
      }
    }
  }
}
void Function::reach_define_analysis() {
  _calc_gen_kill();
  _calc_reach_define_IN_OUT();
}
void Function::_build_lineno_ir_map() {
  lineno_ir_map_.clear();
  int block_num = -1;
  int lineno = -1;
  for (const auto &basic_block : basic_block_vector_) {
    ++block_num;
    basic_block->block_num_ = block_num;
    basic_block->first_lineno_ = lineno + 1;
    for (const auto &ir : *basic_block) {
      ++lineno;
      lineno_ir_map_[lineno] = ir; // 保存每一行对应的ir，加速后续的搜索
    }
    basic_block->last_lineno_ = lineno;
  }
  ir_num_ = lineno;
}
void Function::available_expression_analysis() {
  _calc_egen_ekill();
  _calc_available_expression_IN_OUT();
}
void Function::_calc_egen_ekill() {
  _fill_all_exp_list();
  for (auto &basic_block : basic_block_vector_) {
    basic_block->calc_egen_ekill(all_exp_list_);
  }
}
void Function::_calc_available_expression_IN_OUT() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->available_expression_OUT_ = all_exp_list_;
  }
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_vector_) {
      list<Exp> tmp_IN = all_exp_list_, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &pred_block: basic_block->predecessor_list_) {
        set_intersection(tmp_IN.begin(), tmp_IN.end(),
                         pred_block.lock()->available_expression_OUT_.begin(),
                         pred_block.lock()->available_expression_OUT_.end(),
                         back_inserter(tmp));
        tmp_IN.swap(tmp);
        tmp.clear();
      }
      if (basic_block->predecessor_list_.empty()) {
        basic_block->available_expression_IN_.clear();
      } else {
        basic_block->available_expression_IN_.swap(tmp_IN);
      }
      set_difference(basic_block->available_expression_IN_.begin(),
                     basic_block->available_expression_IN_.end(),
                     basic_block->ekill_.begin(), basic_block->ekill_.end(),
                     back_inserter(tmp_difference_list));
      set_union(basic_block->egen_.begin(), basic_block->egen_.end(),
                tmp_difference_list.begin(), tmp_difference_list.end(),
                back_inserter(tmp_OUT));
      if (basic_block->available_expression_OUT_ != tmp_OUT) {
        change = true;
        basic_block->available_expression_OUT_.swap(tmp_OUT);
      }
    }
  }
}
void Function::_fill_all_exp_list() {
  all_exp_list_.clear();
  auto exist_exp = [&](const Exp &exp) {
    auto result = find(all_exp_list_.begin(), all_exp_list_.end(), exp);
    if (result == all_exp_list_.end()) {
      return false;
    }
    return true;
  };
  for (const auto &basic_block: basic_block_vector_) {
    for (const auto &ir: *basic_block) {
      if (is_algo_op(ir->op_)) {
        auto exp = make_algo_exp(ir);
        if (!exist_exp(exp)) all_exp_list_.push_back(exp);
      } else if (ir->op_ == IR::MOV) {
        auto exp = make_mov_exp(ir);
        if (!exist_exp(exp)) all_exp_list_.push_back(exp);
      }
    }
  }
  all_exp_list_.sort();
}
void Function::live_variable_analysis() {
  _calc_use_def();
//  for (const auto &basic_block: basic_block_list_) {
//    std::cout << red << "block " << basic_block->block_num_ << ":" << normal << std::endl;
//    basic_block->debug();
//    std::cout << blue << "predecessor: " << normal;
//    PRINT_PRED_SUCC_BLOCKS(basic_block->predecessor_list_);
//    std::cout << blue << "successor: " << normal;
//    PRINT_PRED_SUCC_BLOCKS(basic_block->successor_list_);
//    std::cout << blue << "use: " << normal << std::endl;
//    PRINT_ELEMENTS(basic_block->use_);
//    std::cout << blue << "def: " << normal << std::endl;
//    PRINT_ELEMENTS(basic_block->def_);
//  }
  _calc_live_variable_IN_OUT();
}
void Function::_calc_use_def() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->calc_use_def();
  }
}
void Function::_calc_live_variable_IN_OUT() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->live_variable_IN_.clear();
  }
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_vector_) {
      list<IR::Addr> tmp_IN, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &succ_block: basic_block->successor_list_) {
        set_union(tmp_OUT.begin(), tmp_OUT.end(),
                  succ_block.lock()->live_variable_IN_.begin(),
                  succ_block.lock()->live_variable_IN_.end(),
                  back_inserter(tmp));
        tmp_OUT.swap(tmp);
        tmp.clear();
      }
      basic_block->live_variable_OUT_.swap(tmp_OUT);
      set_difference(basic_block->live_variable_OUT_.begin(),
                     basic_block->live_variable_OUT_.end(),
                     basic_block->def_.begin(), basic_block->def_.end(),
                     back_inserter(tmp_difference_list));
      set_union(basic_block->use_.begin(), basic_block->use_.end(),
                tmp_difference_list.begin(), tmp_difference_list.end(),
                back_inserter(tmp_IN));
      if (basic_block->live_variable_IN_ != tmp_IN) {
        change = true;
        basic_block->live_variable_IN_.swap(tmp_IN);
      }
    }
  }
}
void Function::delete_global_common_expression() {
  available_expression_analysis();  // 可用表达式分析
  for (auto &basic_block : basic_block_vector_) {
    for (const auto &exp: basic_block->available_expression_IN_) {
      if (!is_algo_op(exp.op_)) continue; // 只分析算术表达式
      for (const auto &ir: basic_block->ir_list_) {
        if (exp.be_used_by(ir)) {
          auto source_list = _find_sources(exp, basic_block);
          auto assign_exp = make_tmp_assign_exp_ir(exp);
          for (const auto &source : source_list) {
            source.first->ir_list_.insert(source.second, assign_exp);
            auto origin_ir = *source.second;
            origin_ir->op_ = IR::MOV;
            origin_ir->a1 = assign_exp->a0;
            origin_ir->a2 = nullptr;
          }
          ir->op_ = IR::MOV;
          ir->a1 = assign_exp->a0;
          ir->a2 = nullptr;
          break;  // 找到第一条指令就结束查找
        } else if (exp.related_to(ir->a0)) { // 被杀死
          break;
        }
      }
    }
  }
}
void Function::delete_local_common_expression() {
  for (const auto &basic_block : basic_block_vector_) {
    basic_block->delete_local_common_expression();
  }
}
list<Function::source> Function::_find_sources(const Exp &exp, const shared_ptr<BasicBlock> &cur_block) {
  searched_.assign(basic_block_vector_.size(), false);
  list<source> sources;
  _real_find_sources(exp, cur_block, sources);
  return sources;
}
void Function::_real_find_sources(const Exp &exp,
                                  const shared_ptr<BasicBlock> &cur_block,
                                  list<Function::source> &sources) {
  for (auto &pred_block: cur_block->predecessor_list_) {
    const shared_ptr<BasicBlock> &block = pred_block.lock();
    int cur_block_num = block->block_num_;
    if (!searched_[cur_block_num]) {  // 只搜索未搜索过的块
      searched_[cur_block_num] = true;
      bool killed = false, find_flag = false;
      // 从后往前逆向搜索
      for (auto iter = block->ir_list_.rbegin(); iter != block->ir_list_.rend(); ++iter) {
        const auto &cur_ir = *iter;
        if (!is_algo_op(cur_ir->op_)) continue; // 只处理算术指令
        if (exp.related_to(cur_ir->a0)) { // 被杀死了，NOTE: 理论上不会出现这种情况
          killed = true;
          break;
        } else if (exp.be_used_by(cur_ir)) { // 找到了
          sources.emplace_back(block, prev(iter.base()));
          find_flag = true;
          break;
        }
      }
      if (!killed && !find_flag) {  // 没有被杀死，且没有找到
        _real_find_sources(exp, block, sources);
      }
    }
  }
}
void Function::constant_folding() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->constant_folding();
  }
}
void Function::algebraic_simplification() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->algebraic_simplification();
  }
}
void Function::local_copy_propagation() {
  set<Exp> available_copy_exps;
  for (auto &basic_block : basic_block_vector_) {
    available_copy_exps.clear();
    basic_block->local_copy_propagation(available_copy_exps);
  }
}
void Function::global_copy_propagation() {
  available_expression_analysis();
  set<Exp> available_copy_exps;
  for (auto &basic_block : basic_block_vector_) {
    available_copy_exps.clear();
    for (const auto &exp: basic_block->available_expression_IN_) {
      if (exp.op_ == IR::MOV) {
        available_copy_exps.insert(exp);
      }
    }
    basic_block->local_copy_propagation(available_copy_exps);
  }
}
void Function::remove_dead_code() {
  live_variable_analysis();
  for (auto &basic_block : basic_block_vector_) {
    basic_block->remove_dead_code();
  }
}
void Function::_calc_dominate_IN_OUT() {
  list<int> all_block_list;
  for (auto &basic_block : basic_block_vector_) {
    all_block_list.push_back(basic_block->block_num_);
  }
  for (auto &basic_block : basic_block_vector_) {
    basic_block->dominate_OUT_ = all_block_list;
  }
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_vector_) {
      list<int> tmp_IN = all_block_list, tmp, tmp_OUT;
      for (const auto &pred_block: basic_block->predecessor_list_) {
        set_intersection(tmp_IN.begin(), tmp_IN.end(),
                         pred_block.lock()->dominate_OUT_.begin(),
                         pred_block.lock()->dominate_OUT_.end(),
                         back_inserter(tmp));
        tmp_IN.swap(tmp);
        tmp.clear();
      }
      if (basic_block->predecessor_list_.empty()) {
        basic_block->dominate_IN_.clear();
      } else {
        basic_block->dominate_IN_.swap(tmp_IN);
      }
      tmp_OUT = basic_block->dominate_IN_;
      if (find(tmp_OUT.begin(), tmp_OUT.end(),
               basic_block->block_num_) == tmp_OUT.end()) {
        tmp_OUT.push_back(basic_block->block_num_);
      }
      if (basic_block->dominate_OUT_ != tmp_OUT) {
        change = true;
        basic_block->dominate_OUT_.swap(tmp_OUT);
      }
    }
  }
}
void Function::loop_invariant_code_motion() {
  auto insert_preheader = [&](const loop &l, const BasicBlock::Ptr &preheader_block) -> BasicBlock::Ptr {
    auto entry_block = l.front();  // loop entry
    auto entry_block_iter = next(basic_block_vector_.begin(), entry_block->block_num_);
    for (auto iter = entry_block->predecessor_list_.begin(); iter != entry_block->predecessor_list_.end();) {
      auto pred_block = (*iter).lock();
      if (std::find_if(l.begin(), l.end(), [&](const BasicBlock::Ptr &block) {
        return block->block_num_ == pred_block->block_num_;
      }) == l.end()) {  // 不在循环中
        preheader_block->predecessor_list_.push_back(pred_block);
        iter = entry_block->predecessor_list_.erase(iter);
        for (auto it = pred_block->successor_list_.begin(); it != pred_block->successor_list_.end();) {
          if ((*it).lock()->block_num_ == entry_block->block_num_) {
            it = pred_block->successor_list_.erase(it);
          } else {
            ++it;
          }
        }
        pred_block->successor_list_.push_back(preheader_block);
      } else {
        ++iter;
      }
    }
    preheader_block->successor_list_.push_back(entry_block);
    entry_block->predecessor_list_.push_back(preheader_block);
    basic_block_vector_.insert(entry_block_iter, preheader_block);
    return preheader_block;
  };
  auto dom_all_exit = [&](const BasicBlock::Ptr &block, const set<int> &exits) -> bool {
    return all_of(exits.begin(), exits.end(), [&](int e) {
      auto e_block = basic_block_vector_[e];
      if (find(e_block->dominate_OUT_.begin(), e_block->dominate_OUT_.end(), block->block_num_)
          == e_block->dominate_OUT_.end()) {  // 不在exit的支配节点中
        return false;
      }
      return true;
    });
  };
  auto block_use_var = [&](const BasicBlock::Ptr &block, const IR_Addr::Ptr &var) {
    return std::any_of(block->begin(), block->end(), [&](const IR::Ptr &ir) {
      if (is_algo_op(ir->op_)) {
        return *(ir->a1) == *var || *(ir->a2) == *var;
      } else if (is_mov_op(ir->op_)) {
        return *(ir->a1) == *var;
      } else if (ir->op_ == IR::CMP) {
        return *(ir->a1) == *var || *(ir->a2) == *var;
      } else if (ir->op_ == IR::PARAM) {
        return *(ir->a1) == *var;
      } else if (ir->op_ == IR::RET) {
        return *(ir->a0) == *var;
      } else if (ir->op_ == IR::LOAD) {
        return *(ir->a1) == *var || *(ir->a2) == *var;
      } else if (ir->op_ == IR::STORE) {
        return *(ir->a0) == *var || *(ir->a1) == *var || *(ir->a2) == *var;
      }
      return false;
    });
  };
  auto dom_all_use = [&](const BasicBlock::Ptr &block, const loop &l, const IR_Addr::Ptr &a) -> bool {
    return all_of(l.begin(), l.end(), [&](const auto &loop_block) {
      if (block_use_var(loop_block, a)) {
        if (std::find(loop_block->dominate_OUT_.begin(), loop_block->dominate_OUT_.end(), block->block_num_)
            == loop_block->dominate_OUT_.end()) {
          return false;
        }
      }
      return true;
    });
  };
  auto get_all_exit = [&](const loop &l) {
    set<int> all_exit;
    for (const auto &block : l) {
      for (const auto &succ_block : block->successor_list_) {
        int succ_block_num = succ_block.lock()->block_num_;
        if (std::find_if(l.begin(), l.end(), [&](const BasicBlock::Ptr &loop_block) {
          return loop_block->block_num_ == succ_block_num;
        }) == l.end()) {
          all_exit.insert(succ_block_num);
        }
      }
    }
    return all_exit;
  };
  _find_back_edges();
  for (const auto &e : back_edges_) {
    _build_lineno_ir_map(); // update block_num, ir_num_
    auto l = _get_loop(e);
    auto mark_set = _mark_loop_invariant(l);
    if (mark_set.empty()) continue;
    auto preheader = make_empty_basic_block();
    auto all_exit = get_all_exit(l);
    for (auto[cur_block, cur_ir_iter] : mark_set) {
      auto cur_ir = *cur_ir_iter;
      if (dom_all_exit(cur_block, all_exit) && dom_all_use(cur_block, l, cur_ir->a0)) {  // 符合移动条件
        preheader->ir_list_.push_back(cur_ir);
        cur_block->ir_list_.erase(cur_ir_iter); // 移除当前指令
      }
    }
    if (!preheader->ir_list_.empty()) {
      insert_preheader(l, preheader);
    }
  }
}
list<IR::Ptr> Function::merge() {
  list<IR::Ptr> ret;
  for (auto &basic_block : basic_block_vector_) {
    ret.splice(ret.end(), basic_block->ir_list_);
  }
  return ret;
}
void Function::_find_back_edges() {
  back_edges_.clear();
  _calc_dominate_IN_OUT();
  for (const auto &basic_block : basic_block_vector_) {
    for (int dominate_block_num : basic_block->dominate_OUT_) {
      for (const auto &succ_block : basic_block->successor_list_) {
        if (succ_block.lock()->block_num_ == dominate_block_num) {
          back_edges_.insert(make_pair(basic_block, basic_block_vector_[dominate_block_num]));
          break;
        }
      }
    }
  }
}
Function::loop Function::_get_loop(const Function::edge &e) {
  loop ret;
  ret.push_back(e.first);
  ret.push_back(e.second);
  stack<BasicBlock::Ptr> s;
  s.push(e.first);
  while (!s.empty()) {
    auto m = s.top();
    s.pop();
    for (const auto &pred_block_weak : m->predecessor_list_) {
      auto pred_block = pred_block_weak.lock();
      int pred_num = pred_block->block_num_;
      if (std::find_if(ret.begin(), ret.end(), [&](const BasicBlock::Ptr &loop_block) {
        return loop_block->block_num_ == pred_num;
      }) == ret.end()) {
        ret.push_back(pred_block);  // add to loop
        s.push(pred_block);
      }
    }
  }
  std::sort(ret.begin(), ret.end(), [](const BasicBlock::Ptr &lhs, const BasicBlock::Ptr &rhs) {
    return lhs->block_num_ < rhs->block_num_;
  }); // 按照blocknum从小到大排序
  return ret;
}
vector<pair<BasicBlock::Ptr, BasicBlock::iterator>>
Function::_mark_loop_invariant(Function::loop &l) {
  reach_define_analysis();
  vector<pair<BasicBlock::Ptr, BasicBlock::iterator>> ret;  // 所有的循环不变计算
  inst_invariant_vec_.assign(ir_num_, false);
  _build_lineno_rd_vec();
  bool change = true;

  auto is_loop_constant = [](const IR_Addr::Ptr &a) -> bool {
//    assert(a != nullptr);
    return a->kind == IR_Addr::IMM;
  };
  auto lineno_in_loop = [&](int lineno) -> bool {
    return any_of(l.begin(), l.end(), [&](const BasicBlock::Ptr &loop_block) {
      if (lineno >= loop_block->first_lineno_ && lineno <= loop_block->last_lineno_) {
        return true;
      }
      return false;
    });
  };
  auto reach_define_out = [&](const BasicBlock::Ptr &block, const IR_Addr::Ptr &a) -> bool {
    return all_of(block->reach_define_IN_.begin(), block->reach_define_IN_.end(), [&](int in_lineno) {
      auto rd = lineno_rd_vec_[in_lineno];
      if (rd && (*rd == *a)) {  // 是对a的定值
        if (lineno_in_loop(in_lineno)) {
          return false;
        }
      }
      return true;
    }); // 所有到达基本块block的对a的定值都位于循环之外
  };
  auto reach_define_in = [&](const BasicBlock::Ptr &block, const IR_Addr::Ptr &a, int cur_lineno) -> bool {
    // a肯定不为立即数
    vector<int> tmp;
    for (int in_lineno : block->reach_define_IN_) {
      auto rd = lineno_rd_vec_[in_lineno];
      if (rd && (*rd == *a)) {  // 是对a的定值
        if (lineno_in_loop(in_lineno)) {
          if (!inst_invariant_vec_[in_lineno]) return false;  // 不是循环不变的
          if (in_lineno > cur_lineno) return false; // 不在当前指令之前执行
          tmp.push_back(in_lineno);
        }
      }
    }
    if (tmp.size() != 1) return false;
    int lineno = block->first_lineno_;
    auto cur_rd = *lineno_rd_vec_[cur_lineno];
    for (const auto &ir : block->ir_list_) {
      if (lineno >= cur_lineno) return true;
      if (ir->a1 && *(ir->a1) == cur_rd) {
        return false; // 存在对结果变量的使用
      }
      if (ir->a2 && *(ir->a2) == cur_rd) {
        return false; // 存在对结果变量的使用
      }
    }
    return true;  // unreachable!
  };
  while (change) {
    change = false;
    for (const auto &cur_block : l) {
      auto cur_lineno = cur_block->first_lineno_;
      for (auto cur_iter = cur_block->begin(); cur_iter != cur_block->end(); ++cur_iter) {
        auto ir = *cur_iter;
        if (is_algo_op(ir->op_)) {  // 两个源操作数的算术指令
          if (!inst_invariant_vec_[cur_lineno]) { // 当前指令不是循环不变计算
            bool a1_const = false, a2_const = false;
            if (is_loop_constant(ir->a1) || reach_define_out(cur_block, ir->a1)
                || reach_define_in(cur_block, ir->a1, cur_lineno)) {
              a1_const = true;
            }
            if (is_loop_constant(ir->a2) || reach_define_out(cur_block, ir->a2)
                || reach_define_in(cur_block, ir->a2, cur_lineno)) {
              a2_const = true;
            }
            if (a1_const && a2_const) {  // 新增了循环不变计算
              inst_invariant_vec_[cur_lineno] = true;
              ret.emplace_back(cur_block, cur_iter);
              change = true;
            }
          }
        } else if (is_mov_op(ir->op_)) {
          if (!inst_invariant_vec_[cur_lineno]) { // 当前指令不是循环不变计算
            bool a1_const = false;
            if (is_loop_constant(ir->a1) || reach_define_out(cur_block, ir->a1)
                || reach_define_in(cur_block, ir->a1, cur_lineno)) {
              a1_const = true;
            }
            if (a1_const) {  // 新增了循环不变计算
              inst_invariant_vec_[cur_lineno] = true;
              ret.emplace_back(cur_block, cur_iter);
              change = true;
            }
          }
        }
        ++cur_lineno;
      }
    }
  }
  return ret;
}
void Function::_build_lineno_rd_vec() {
  lineno_rd_vec_.assign(ir_num_, nullptr);
  for (const auto &basic_block : basic_block_vector_) {
    auto cur_lineno = basic_block->first_lineno_;
    for (const auto &ir : basic_block->ir_list_) {
      if (is_algo_op(ir->op_) || is_mov_op(ir->op_)) {
        lineno_rd_vec_[cur_lineno] = ir->a0;
      }
      ++cur_lineno;
    }
  }
}
void Function::ir_specify_optimization() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->ir_specify_optimization();
  }
}
void Function::_update_blocknum() {
  int cur_blocknum = 0;
  for (auto &block : basic_block_vector_) {
    block->block_num_ = cur_blocknum;
    ++cur_blocknum;
  }
}

void Function::_explore(const BasicBlock::Ptr &block) {
  for (const auto &succ_block_weak : block->successor_list_) {
    auto succ_block = succ_block_weak.lock();
    if (!reachable_[succ_block->block_num_]) {
      reachable_[succ_block->block_num_] = true;
      _explore(succ_block);
    }
  }
}

void Function::delete_unreachable_code() {
  bool again = true;
  reachable_.assign(basic_block_vector_.size(), false);
  reachable_[0] = true;
  _explore(basic_block_vector_[0]);
  int delete_num = 0, sz = basic_block_vector_.size();
  for (int i = 0; i < sz; ++i) {
    if (!reachable_[i]) {
      _delete_block(i - delete_num);
      ++delete_num;
    }
  }
}

void Function::_delete_block(int i) {
  auto delete_iter = basic_block_vector_.begin() + i;
  auto delete_block = *delete_iter;
  for (auto &succ_block_weak : delete_block->successor_list_) {
    auto succ_block = succ_block_weak.lock();
    auto iter = std::find_if(succ_block->predecessor_list_.begin(),
                             succ_block->predecessor_list_.end(),
                             [&](const BasicBlock::Ptr_weak &succ_pred_block) {
                               return succ_pred_block.lock()->block_num_ == delete_block->block_num_;
                             });
    if (iter != succ_block->predecessor_list_.end()) {
      succ_block->predecessor_list_.erase(iter);
    }
  }
  for (auto &pred_block_weak : delete_block->predecessor_list_) {
    auto pred_block = pred_block_weak.lock();
    auto iter = std::find_if(pred_block->successor_list_.begin(),
                             pred_block->successor_list_.end(),
                             [&](const BasicBlock::Ptr_weak &pred_succ_block) {
                               return pred_succ_block.lock()->block_num_ == delete_block->block_num_;
                             });
    if (iter != pred_block->predecessor_list_.end()) {
      pred_block->predecessor_list_.erase(iter);
    }
  }
  basic_block_vector_.erase(delete_iter);
}
void Function::staighten() {
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_vector_) {
      assert(!basic_block->ir_list_.empty()); // DELETE: need to avoid
      auto last_ir = basic_block->ir_list_.back();
      if (!is_jmp_op(last_ir->op_)) continue;
      if (len_of_list(basic_block->successor_list_) == 1) {
        auto succ_block = basic_block->successor_list_.front().lock();
        // DECIDE: 是否检查succ_block的第一句为label
        if (len_of_list(succ_block->predecessor_list_) == 1) {
          auto succ_pred_block = succ_block->predecessor_list_.front().lock();
          assert(succ_pred_block->block_num_ == basic_block->block_num_); // DELETE: 理论上必然成立
          _merge_block(basic_block, succ_block);
          change = true;
          break;  // 修改了容器，如果再循环可能会出问题
        }
      }
    }
  }
  // DECIDE: delete_unreachable_code here
}

void Function::_merge_block(const BasicBlock::Ptr &block1, const BasicBlock::Ptr &block2) {
  block1->ir_list_.pop_back();  // pop jmp op (JMP-JNE)
  block2->ir_list_.pop_front(); // pop label
  block1->ir_list_.splice(block1->ir_list_.end(),
                          block2->ir_list_,
                          block2->ir_list_.begin(), block2->ir_list_.end());
  block1->successor_list_ = block2->successor_list_;
  auto block2_iter = basic_block_vector_.begin() + block2->block_num_;
  basic_block_vector_.erase(block2_iter);
  _build_lineno_ir_map();
}

void Function::if_simplify() {
  for (auto &basic_block : basic_block_vector_) {
    basic_block->if_simplify();
  }
  _rebuild_basic_block();
}

void Function::_rebuild_basic_block() {
  auto ir_list = merge();
  _divide_basic_block(ir_list);
  _build_lineno_ir_map();
  _link_basic_block();
}

void Function::tail_merging() { // TODO: ignore jmp, ir_specify_optimization
  auto equal_of_ir_addr_ptr = [](const IR::Addr::Ptr &a, const IR::Addr::Ptr &b) {
    if (a == nullptr) return b == nullptr;
    if (b == nullptr) return a == nullptr;
    if (a->kind != b->kind) return false;
    if (a->kind == IR_Addr::VAR || a->kind == IR_Addr::PARAM ||
        a->kind == IR_Addr::IMM || a->kind == IR_Addr::BRANCH_LABEL) {
      return a->val == b->val;
    } else if (a->kind == IR_Addr::NAMED_LABEL) {
      return a->name == b->name;
    }
    return true;
  };
  auto equal_of_ir = [&equal_of_ir_addr_ptr](const IR::Ptr &a, const IR::Ptr &b) {
    if (a->op_ != b->op_) return false;
    return equal_of_ir_addr_ptr(a->a0, b->a0) && equal_of_ir_addr_ptr(a->a1, b->a1) &&
        equal_of_ir_addr_ptr(a->a2, b->a2);
  };
  for (auto iter = basic_block_vector_.rbegin(); iter != basic_block_vector_.rend(); ++iter) {
    auto cur_block = *iter;
    auto pred_block_num = len_of_list(cur_block->predecessor_list_);
    auto continue_flag = false;
    if (pred_block_num > 1) {  // 多前驱
      std::vector<BasicBlock::reverse_iterator> ir_iter_vec;
      std::vector<BasicBlock::iterator> ir_iter_end_vec;
      std::vector<BasicBlock::Ptr> pred_block_vec;
      for (const auto &pred_block_weak : cur_block->predecessor_list_) {
        auto pred_block = pred_block_weak.lock();
        auto rbeg = pred_block->ir_list_.rbegin();
        auto last_ir = *rbeg;
        if (is_jmp_op(last_ir->op_) && last_ir->op_ != IR::Op::JMP) { // 不是无条件跳转
          continue_flag = true;
          break;
        } else if (last_ir->op_ == IR::Op::JMP) {
          ++rbeg; // 比较的时候忽略JMP语句
        }
        ir_iter_vec.push_back(rbeg);
        ir_iter_end_vec.push_back(rbeg.base());
        pred_block_vec.push_back(pred_block);
      }
      if (continue_flag) continue;  // 包含有条件跳转，不符合要求
      int same_ir_num = 0;
      while (true) {
        IR::Ptr pre_ir = nullptr;
        bool break_flag = false;
        int i = -1;
        for (auto &ir_iter : ir_iter_vec) {
          ++i;
          if (ir_iter == pred_block_vec[i]->ir_list_.rend()) {  // 扫描完某个前驱块的IR
            break_flag = true;
            for (int j = 0; j < i; ++j) { // 恢复前面的迭代器
              --ir_iter_vec[j];
            }
            break;
          }
          if (pre_ir == nullptr) continue;
          if (!equal_of_ir(pre_ir, *ir_iter)) { // 两条指令不相等
            break_flag = true;
            for (int j = 0; j < i; ++j) { // 恢复前面的迭代器
              --ir_iter_vec[j];
            }
            break;
          }
          pre_ir = *ir_iter;
          ++ir_iter;
        }
        if (break_flag) break;
        ++same_ir_num;
      }
      if (same_ir_num > 0) {  // 有相同的ir
        auto insert_iter = cur_block->ir_list_.begin();
        ++insert_iter;  // 前进到label之后
        auto first_pred_block = pred_block_vec.front();
        cur_block->ir_list_.splice(insert_iter,
                                   first_pred_block->ir_list_,
                                   ir_iter_vec.front().base(), ir_iter_end_vec[0]);
        for (int i = 1; i < pred_block_num; ++i) {  // 把其他块后面的IR指令删除
          pred_block_vec[i]->ir_list_.erase(ir_iter_vec[i].base(), ir_iter_end_vec[i]);
        }
      }
    }
  }
}

void Function::strength_reduction() {
  // TODO
}

void Function::induction_variable_elimination() {
  // TODO
}

void Function::optimize(int optimize_level) {
  // TODO
}
