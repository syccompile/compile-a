#include "basicblock.h"

const std::string normal = "\033[0m";
const std::string black = "\033[0;30m";
const std::string red = "\033[0;31m";
const std::string green = "\033[0;32m";
const std::string yellow = "\033[0;33m";
const std::string blue = "\033[0;34m";
const std::string white = "\033[0;37m";

bool operator==(const IR::Addr &lhs, const IR::Addr &rhs) {
  return lhs.kind == rhs.kind && lhs.val == rhs.val;
}

bool operator<(const IR::Addr &lhs, const IR::Addr &rhs) {
  return lhs.kind < rhs.kind || lhs.val < rhs.val;
}

std::ostream& operator<<(std::ostream& os, IR::Addr a) {
  a.internal_print();
  return os;
}

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

bool is_mov_op(IR::Op op) {
  return op >= IR::Op::MOV && op <= IR::Op::MOVNE;
}

bool is_algo_op(IR::Op op) {
  return op >= IR::Op::ADD && op <= IR::Op::MOD;
}

/* 通过ir_list新建一个BasicBlock, 返回指向它的shared_ptr */
std::shared_ptr<BasicBlock> make_basic_block(const std::list<IR::Ptr> &ir_list) {
  return std::make_shared<BasicBlock>(ir_list);
}

std::shared_ptr<BasicBlock> make_empty_basic_block() {
  return make_basic_block(std::list<IR::Ptr>());
}

/* 通过ir_list新建一个FunctionBlock, 返回指向它的shared_ptr */
std::shared_ptr<Function>
make_function_block(std::list<IR::Ptr> &ir_list) {
  return std::make_shared<Function>(ir_list);
}

inline Exp make_algo_exp(const IR::Ptr &ir) {
  return Exp(ir->op_, ir->a1, ir->a2);
}

inline Exp make_mov_exp(const IR::Ptr &ir) {
  return Exp(ir->op_, ir->a0, ir->a1);
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

  auto delete_egen_exp = [&](IR::Addr::Ptr a) { // 删除和a相关的表达式
    for (auto iter = egen_.begin(); iter != egen_.end(); ++iter) {
      auto cur_exp = *iter;
      if (cur_exp.related_to(a)) {
        iter = egen_.erase(iter);
      } // ignore else
    }
  };
  auto delete_ekill_exp = [&](const Exp &exp) {
    for (auto iter = ekill_.begin(); iter != ekill_.end(); ++iter) {
      if (*iter == exp) {
        iter = ekill_.erase(iter);
      }
    }
  };
  auto add_ekill_exp = [&](const IR::Addr::Ptr &a) {
    for (const auto &exp: all_exp_list) {
      if (exp.related_to(a)) {
        ekill_.push_back(exp);
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
      if (ir->op_ == IR::Op::MOV) { // 只有无条件赋值语句才将表达式加入
        auto new_exp = make_mov_exp(ir);
        egen_.push_back(new_exp);
      }
      // 每一种赋值语句都需要把表达式删除
      delete_egen_exp(ir->a0);
      delete_egen_exp(ir->a0);
      add_ekill_exp(ir->a0);
    } // 不处理LOAD指令
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
    }
  }
  use_.sort();
  def_.sort();
}
void BasicBlock::delete_local_common_expression() {
  std::map<Exp, IR::Addr::Ptr> available_exps;
  auto delete_avail_exp = [&](const IR::Addr::Ptr &a) {
    for (auto iter = available_exps.begin(); iter != available_exps.end(); )
      if ((*iter).first.related_to(a)) {
        iter = available_exps.erase(iter);
      } else {
        ++iter;
      }
  };

  for (const auto& cur_ir : ir_list_) {
    if (is_algo_op(cur_ir->op_)) {
      auto exp = make_algo_exp(cur_ir);
      auto result = available_exps.find(exp);
      if (result != available_exps.end()) {
        cur_ir->op_ = IR::Op::MOV;
        cur_ir->a1 = result->second;
        cur_ir->a2 = nullptr;
      } else {
        available_exps[exp] = cur_ir->a0; // add avail_exp
      }
      delete_avail_exp(cur_ir->a0);
    } else if (is_mov_op(cur_ir->op_)) {
      delete_avail_exp(cur_ir->a0);
    }
  }
}

Function::Function(std::list<IR::Ptr> &ir_list) {
  func_name_ = ir_list.front()->a0->name;
  arg_num_ = ir_list.front()->a1->val;
  ir_list.pop_front();  // pop FUNCDEF

  auto tmp_basic_block = make_empty_basic_block();  // 第一条指令是首指令
  while (!(ir_list.empty())) {
    auto front = ir_list.front();
    if (front->op_ == IR::Op::LABEL) {
      if (!(tmp_basic_block->ir_list_.empty())) {
        basic_block_list_.push_back(tmp_basic_block);
        tmp_basic_block = make_empty_basic_block(); // 跳转指令的目标指令是首指令
      }
      tmp_basic_block->ir_list_.push_back(front);
    } else if ((front->op_ >= IR::Op::JMP && front->op_ <= IR::Op::JNE) ||
        (front->op_ == IR::Op::RET)) {
      tmp_basic_block->ir_list_.push_back(front);
      basic_block_list_.push_back(tmp_basic_block);
      tmp_basic_block = make_empty_basic_block(); // 跳转指令的后一条指令是首指令
    } else {
      tmp_basic_block->ir_list_.push_back(front);
    }
    ir_list.pop_front();
  }
  if (!(tmp_basic_block->ir_list_.empty())) {
    basic_block_list_.push_back(tmp_basic_block);
  }

  // TODO:下面的代码导致重复更新block_num，也许需要改进，虽然复杂度并不高
  int block_num = -1;
  for (const auto &basic_block : basic_block_list_) {
    ++block_num;
    basic_block->block_num_ = block_num;
  }

  // 计算前驱节点和后继节点
  auto find_label = [&](int label_num) {
    for (const auto &basic_block : basic_block_list_) {
      auto first_ir = basic_block->ir_list_.front();
      if (first_ir->op_ == IR::Op::LABEL && first_ir->a0->val == label_num) {
        return basic_block;
      }
    }
    throw std::string("could not find label ") + std::to_string(label_num);
  };

  for (auto iter = basic_block_list_.begin(); iter != basic_block_list_.end(); ++iter) {
    auto basic_block = *iter;
    auto last_ir = basic_block->ir_list_.back();
    try {
      if (last_ir->op_ == IR::Op::JMP) {  // 无条件跳转
        auto target_block = find_label(last_ir->a0->val);
        basic_block->successor_list_.push_back(target_block);
        target_block->predecessor_list_.push_back(basic_block);
      } else if (last_ir->op_ == IR::Op::RET) { // 无条件跳转
        if (std::next(iter) != basic_block_list_.end()) {
          auto next_block = *std::next(iter);
          basic_block->successor_list_.push_back(next_block);
          next_block->predecessor_list_.push_back(basic_block);
        }
      } else if (last_ir->op_ >= IR::Op::JLE && last_ir->op_ <= IR::Op::JNE) {  // 条件跳转
        auto next_block = *std::next(iter);
        auto target_block = find_label(last_ir->a0->val);
        if (next_block->block_num_ != target_block->block_num_) {
          basic_block->successor_list_.push_back(next_block);
          next_block->predecessor_list_.push_back(basic_block);
        } // 两个目标相同则只建立前驱-后继关系一次，不确保该情况会出现
        basic_block->successor_list_.push_back(target_block);
        target_block->predecessor_list_.push_back(basic_block);
      } // ignore else
    } catch (const std::string &e) {
      std::cout << e << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}
std::list<std::string> Function::translate_to_arm() {
  return std::list<std::string>();
}
void Function::debug() {
  std::cout << green << func_name_ << ": " << normal << std::endl;
  std::cout << "------------------------" << std::endl;
//  _calc_gen_kill();
  reach_define_analysis();
  available_expression_analysis();
  live_variable_analysis();
  // delete_local_common_expression();
  for (const auto &basic_block : basic_block_list_) {
    std::cout << blue << "block " << basic_block->block_num_ << ":" << normal << std::endl;
    basic_block->debug();
    std::cout << blue << "predecessor: " << normal;
    PRINT_PRED_SUCC_BLOCKS(basic_block->predecessor_list_);
    std::cout << blue << "successor: " << normal;
    PRINT_PRED_SUCC_BLOCKS(basic_block->successor_list_);
    std::cout << blue << "gen: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->gen_);
    std::cout << blue << "kill: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->kill_);
    std::cout << blue << "reach_define_IN: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->reach_define_IN_);
    std::cout << blue << "reach_define_OUT: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->reach_define_OUT_);
    std::cout << blue << "egen: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->egen_);
    std::cout << blue << "ekill: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->ekill_);
    std::cout << blue << "available_expression_IN_: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->available_expression_IN_);
    std::cout << blue << "available_expression_OUT_: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->available_expression_OUT_);
    std::cout << blue << "use: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->use_);
    std::cout << blue << "def: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->def_);
    std::cout << blue << "live_variable_IN_: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->live_variable_IN_);
    std::cout << blue << "live_variable_OUT_: " << normal << std::endl;
    PRINT_ELEMENTS(basic_block->live_variable_OUT_);
    std::cout << std::endl;
  }
//  std::cout << "gen_map: " << std::endl;
//  PRINT_MAP(gen_map_);
//  std::cout << "kill_map: " << std::endl;
//  PRINT_MAP(kill_map_);
  std::cout << "------------------------\n" << std::endl;
}

void Function::_build_gen_kill_map() {
  gen_kill_help_map_.clear();
  gen_map_.clear();
  kill_map_.clear();
  int lineno = -1;
  for (const auto &basic_block : basic_block_list_) {
    for (const auto &ir : *basic_block) {
      ++lineno;
      _add_to_gen_kill_help_map(ir, lineno);
    }
  }
  lineno = -1;
  for (const auto &basic_block : basic_block_list_) {
    for (const auto &ir : *basic_block) {
      ++lineno;
      _add_to_gen_map(ir, lineno);
      _add_to_kill_map(ir, lineno);
    }
  }
}

void Function::_add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::Op::LOAD)) {
    if (ir->a0->kind == IR::Addr::Kind::PARAM) {  // 函数参数
      gen_kill_help_map_.insert(decltype(gen_kill_help_map_)::value_type(ir->a0->val, lineno));
    } else {  // 普通变量
      gen_kill_help_map_.insert(decltype(gen_kill_help_map_)::value_type(ir->a0->val + arg_num_, lineno));
    }
  } // ignore else
}
void Function::_add_to_gen_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::Op::LOAD)) {
    gen_map_[lineno].push_back(lineno);
  } // ignore else
}
void Function::_add_to_kill_map(const IR::Ptr &ir, int lineno) {
  if (is_algo_op(ir->op_) || is_mov_op(ir->op_) || (ir->op_ == IR::Op::LOAD)) {
    int val = ir->a0->val;
    decltype(gen_kill_help_map_)::iterator val_beg, val_end;
    if (ir->a0->kind == IR::Addr::Kind::PARAM) {  // 函数参数
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
  for (const auto &basic_block : basic_block_list_) {
    basic_block->gen_.clear();
    basic_block->kill_.clear();
    std::list<int> gen_list, kill_list, tmp_gen_list, tmp_kill_list, tmp_difference_list;
    for (int cur_lineno = basic_block->last_lineno_; cur_lineno >= basic_block->first_lineno_; --cur_lineno) {
      std::set_difference(gen_map_[cur_lineno].begin(), gen_map_[cur_lineno].end(),
                          kill_list.begin(), kill_list.end(),
                          std::back_inserter(tmp_difference_list));
      std::set_union(gen_list.begin(), gen_list.end(),
                     tmp_difference_list.begin(), tmp_difference_list.end(),
                     std::back_inserter(tmp_gen_list));
      std::set_union(kill_list.begin(), kill_list.end(),
                     kill_map_[cur_lineno].begin(), kill_map_[cur_lineno].end(),
                     std::back_inserter(tmp_kill_list));
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
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_list_) {
      std::list<int> tmp_IN, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &pred_block: basic_block->predecessor_list_) {
        std::set_union(tmp_IN.begin(), tmp_IN.end(),
                       pred_block.lock()->reach_define_OUT_.begin(),
                       pred_block.lock()->reach_define_OUT_.end(),
                       std::back_inserter(tmp));
        tmp_IN.swap(tmp);
        tmp.clear();
      }
      basic_block->reach_define_IN_.swap(tmp_IN);
      std::set_difference(basic_block->reach_define_IN_.begin(),
                          basic_block->reach_define_IN_.end(),
                          basic_block->kill_.begin(), basic_block->kill_.end(),
                          std::back_inserter(tmp_difference_list));
      std::set_union(basic_block->gen_.begin(), basic_block->gen_.end(),
                     tmp_difference_list.begin(), tmp_difference_list.end(),
                     std::back_inserter(tmp_OUT));
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
  for (const auto &basic_block : basic_block_list_) {
    ++block_num;
    basic_block->block_num_ = block_num;
    basic_block->first_lineno_ = lineno + 1;
    for (const auto &ir : *basic_block) {
      ++lineno;
      lineno_ir_map_[lineno] = ir; // 保存每一行对应的ir，加速后续的搜索
    }
    basic_block->last_lineno_ = lineno;
  }
}
void Function::available_expression_analysis() {
  _calc_egen_ekill();
  _calc_available_expression_IN_OUT();
}
void Function::_calc_egen_ekill() {
  _fill_all_exp_list();
  for (auto &basic_block : basic_block_list_) {
    basic_block->calc_egen_ekill(all_exp_list_);
  }
}
void Function::_calc_available_expression_IN_OUT() {
  for (auto &basic_block : basic_block_list_) {
    basic_block->available_expression_OUT_ = all_exp_list_;
  }
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_list_) {
      std::list<Exp> tmp_IN = all_exp_list_, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &pred_block: basic_block->predecessor_list_) {
        std::set_intersection(tmp_IN.begin(), tmp_IN.end(),
                              pred_block.lock()->available_expression_OUT_.begin(),
                              pred_block.lock()->available_expression_OUT_.end(),
                              std::back_inserter(tmp));
        tmp_IN.swap(tmp);
        tmp.clear();
      }
      if (basic_block->predecessor_list_.empty()) {
        basic_block->available_expression_IN_.clear();
      } else {
        basic_block->available_expression_IN_.swap(tmp_IN);
      }
      std::set_difference(basic_block->available_expression_IN_.begin(),
                          basic_block->available_expression_IN_.end(),
                          basic_block->ekill_.begin(), basic_block->ekill_.end(),
                          std::back_inserter(tmp_difference_list));
      std::set_union(basic_block->egen_.begin(), basic_block->egen_.end(),
                     tmp_difference_list.begin(), tmp_difference_list.end(),
                     std::back_inserter(tmp_OUT));
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
    auto result = std::find(all_exp_list_.begin(), all_exp_list_.end(), exp);
    if (result == all_exp_list_.end()) {
      return false;
    }
    return true;
  };
  for (const auto &basic_block: basic_block_list_) {
    for (const auto &ir: *basic_block) {
      if (is_algo_op(ir->op_)) {
        auto exp = make_algo_exp(ir);
        if (!exist_exp(exp))  all_exp_list_.push_back(exp);
      } else if (ir->op_ == IR::Op::MOV){
        auto exp = make_mov_exp(ir);
        if (!exist_exp(exp)) all_exp_list_.push_back(exp);
      }
    }
  }
  all_exp_list_.sort();
}
void Function::live_variable_analysis() {
  _calc_use_def();
  _calc_live_variable_IN_OUT();
}
void Function::_calc_use_def() {
  for (auto &basic_block : basic_block_list_) {
    basic_block->calc_use_def();
  }
}
void Function::_calc_live_variable_IN_OUT() {
  bool change = true;
  while (change) {
    change = false;
    for (auto &basic_block : basic_block_list_) {
      std::list<IR::Addr> tmp_IN, tmp, tmp_OUT, tmp_difference_list;
      for (const auto &succ_block: basic_block->successor_list_) {
        std::set_union(tmp_OUT.begin(), tmp_OUT.end(),
                       succ_block.lock()->live_variable_IN_.begin(),
                       succ_block.lock()->live_variable_IN_.end(),
                       std::back_inserter(tmp));
        tmp_OUT.swap(tmp);
        tmp.clear();
      }
      basic_block->live_variable_OUT_.swap(tmp_OUT);
      std::set_difference(basic_block->live_variable_OUT_.begin(),
                          basic_block->live_variable_OUT_.end(),
                          basic_block->def_.begin(), basic_block->def_.end(),
                          std::back_inserter(tmp_difference_list));
      std::set_union(basic_block->use_.begin(), basic_block->use_.end(),
                     tmp_difference_list.begin(), tmp_difference_list.end(),
                     std::back_inserter(tmp_IN));
      if (basic_block->live_variable_IN_ != tmp_IN) {
        change = true;
        basic_block->live_variable_IN_.swap(tmp_IN);
      }
    }
  }
}
void Function::delete_global_common_expression() {
  std::map<Exp, IR::Addr::Ptr> trace;
  for (auto &basic_block : basic_block_list_) {

  }
}
void Function::delete_local_common_expression() {
  for (const auto &basic_block : basic_block_list_) {
    basic_block->delete_local_common_expression();
  }
}

Module::Module(std::list<IR::Ptr> &ir_list) {
  while (!ir_list.empty()) {
    while (!ir_list.empty() && (ir_list.front()->op_ != IR::Op::FUNCDEF)) {
      ir_list.pop_front();  // 忽略全局声明语句
    }
    if (ir_list.empty()) return;

    auto iter = ir_list.begin();
    while ((*iter)->op_ != IR::Op::FUNCEND) ++iter;
    std::list<IR::Ptr> function_ir_code;
    function_ir_code.splice(function_ir_code.end(),
                            ir_list,
                            ir_list.begin(), iter);
    function_list_.push_back(make_function_block(function_ir_code));
  }
}

std::list<std::string> Module::translate_to_arm() {
  std::list<std::string> ret;
  for (const auto &function : function_list_) {
    ret.splice(ret.end(), function->translate_to_arm());
  }
  return ret;
}
void Module::debug() {
  for (auto &function : function_list_) {
    function->debug();
  }
}
void Module::reach_define_analysis() {
  for (auto &function: function_list_) {
    function->reach_define_analysis();
  }
}
void Module::available_expression_analysis() {
  for (auto &function: function_list_) {
    function->available_expression_analysis();
  }
}
void Module::live_variable_analysis() {
  for (auto &function: function_list_) {
    function->live_variable_analysis();
  }
}
