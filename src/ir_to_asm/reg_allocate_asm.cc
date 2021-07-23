#include "../reg_allocate/liveness_analyze.h"
#include "../ast.h"

#include <stdarg.h>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <assert.h>

// reg alias 
//
// r11 : fp
// r12 : ip
// r13 : sp
// r14 : lr
// r15 : pc

using std::string;
using std::unordered_map;
using std::set;
namespace {
// 总共有15个寄存器
const int kReg_Num = 15;

// 最多可以使用r0-r11的寄存器
const int kMax_Use_Reg = 11;

// 前4个参数使用寄存器传递
const int kTransfer_Reg = 4;

set<IR_Addr::Ptr> _mark_set;

string regs_name(int index) { return string("r") + to_string(index); }

// 获取regs中特定color的寄存器
int regs_find_same_color(IR::Addr::Ptr regs[kMax_Use_Reg], color c) {
  for (int i = 0; i < kMax_Use_Reg; ++i) {
    if (regs[i]->get_color() == c) {
      return i;
    }
  }
  return -1;
}

void mark_addr(IR_Addr::Ptr addr) { _mark_set.insert(addr); }

void unmark_addr(IR_Addr::Ptr addr) { _mark_set.erase(addr); }

bool is_marked(IR_Addr::Ptr addr) {
  return _mark_set.count(addr) > 0;
}

// 获取regs中一个空寄存器或者溢出变量的寄存器,
// 如果找不到返回-1
int regs_find_null_or_spill(IR::Addr::Ptr regs[kMax_Use_Reg]) {
  for (int i = 0; i < kMax_Use_Reg; ++i) {
    if (regs[i] == nullptr || regs[i]->get_color() == spill_color()) {
      return i;
    }
  }
  return -1;
}

// 获取regs中一个特定的addr
int regs_find_addr(IR::Addr::Ptr regs[kMax_Use_Reg], IR::Addr::Ptr addr) {
  for (int i = 0; i < kMax_Use_Reg; ++i) {
    if (regs[i] == addr) {
      return i;
    }
  }
  return -1;
}

// 根据某种策略选择一个寄存器
int chose_a_reg(IR::Addr::Ptr regs[kMax_Use_Reg]) {
  for (int i = 0; i < kMax_Use_Reg; ++i) {
    if (!is_marked(regs[i])) {
      return i;
    }
  }
  return -1;
}

// pre-condition: addr不能是溢出的
std::list<string>
load_addr_to_reg(IR::Addr::Ptr addr, IR::Addr::Ptr regs[kMax_Use_Reg],
                 const unordered_map<IR::Addr::Ptr, int> sp_offset_map,
                 unordered_map<IR::Addr::Ptr, int> addr2reg_map) {
  std::list<string> ret;
  // 无色, 不加载到寄存器
  if (addr->get_color() == none_color()) {
    return ret;
  }

  if (addr->get_color() == spill_color()) {
    // 该变量是溢出的
    int null_reg = regs_find_null_or_spill(regs);
    if (null_reg >= 0) {
      {
        // 如果存在空的或存储溢出变量的寄存器，加载到该寄存器上
        regs[null_reg] = addr;
        int offset  =sp_offset_map.at(addr);
        ret.emplace_back(string("ldr\t") + regs_name(null_reg) + ", [sp], #" + to_string(offset));
        addr2reg_map[addr] = null_reg;
      }
    } else {
      // 如果不存在空寄存器
      // 选择一个寄存器
      // 注意 对于一条指令,例如add, 有三个变量，分别为
      // a0, a1, a2, 如果a2是溢出的，需要加载到寄存器中
      // 不能将a0, a1写回，具体依赖于chose_a_reg()的实现
      int reg_num = chose_a_reg(regs);

      // 将该寄存器上相同颜色的变量写回栈, 两变量颜色一样
      // 表明其共用一个寄存器, 意味着其值是相同的, 写回时
      // 需要将一个寄存器的值写入到对应的变量它们各自的地址中
      auto it = addr2reg_map.begin();
      while (it != addr2reg_map.end()) {
        if (it->first->get_color() == regs[reg_num]->get_color()) {
          int offset = sp_offset_map.at(it->first);
          ret.emplace_back(string("str\t") + regs_name(reg_num) + ", [sp], #" +
                           to_string(offset));
          it = addr2reg_map.erase(it);
        }
        it++;
      }
      // 加载新的寄存器值,
      // 将变量的值加载到寄存器上
      regs[reg_num] = addr;
      int offset = sp_offset_map.at(addr);
      ret.emplace_back(string("ldr\t") + regs_name(reg_num) + ", [sp], #" +
                       to_string(offset));
      addr2reg_map[addr] = reg_num;
    }
  } else if (addr2reg_map.count(addr)) {
    // addr已经在reg中, 无需操作
  } else {
    // addr不是溢出的, 而且不在寄存器中
    int reg_num = regs_find_same_color(regs, addr->get_color());
    // 尝试找一个相同颜色的寄存器, 如果找到了可以直接将值加载上去
    // 如果找不到尝试找一个空或存储溢出变量的寄存器, 必然可以找到，
    // 因为该变量不溢出
    bool find_same_color = reg_num >= 0;
    if (find_same_color) {

      // 加载新的寄存器值
      regs[reg_num] = addr;
      int offset  =sp_offset_map.at(addr);
      ret.emplace_back(string("ldr\t") 
          + regs_name(reg_num) + ", [sp], #" + to_string(offset));
      addr2reg_map[addr] = reg_num;

    }else{
      reg_num = regs_find_null_or_spill(regs);
      // 不可能找不到一个空的寄存器因为addr是着色节点
      assert(reg_num >= 0);

      // 如果找到一个溢出变量的寄存器，写回
      if (regs[reg_num]) {
        int offset = sp_offset_map.at(regs[reg_num]);
        ret.emplace_back(string("str\t") + regs_name(reg_num) + ", [sp], #" +
                         to_string(offset));
        addr2reg_map.erase(regs[reg_num]);
      }
    }
  }
  return ret;
}

#define OP_CASE(op)  case IR::Op::op:
#define THREE_ADDR_ONE_TO_ONE(case, asm_op)                                    \
  OP_CASE(case) {                                                              \
    mark_addr(ir->a0);                                                         \
    mark_addr(ir->a1);                                                         \
    mark_addr(ir->a2);                                                         \
    ret.splice(ret.end(),                                                      \
               load_addr_to_reg(ir->a1, regs, sp_offset_map, addr2reg_map));   \
    ret.splice(ret.end(),                                                      \
               load_addr_to_reg(ir->a2, regs, sp_offset_map, addr2reg_map));   \
    ret.splice(ret.end(),                                                      \
               load_addr_to_reg(ir->a0, regs, sp_offset_map, addr2reg_map));   \
    unmark_addr(ir->a0);                                                       \
    unmark_addr(ir->a1);                                                       \
    unmark_addr(ir->a2);                                                       \
    assert(addr2reg_map.count(ir->a0));                                        \
    assert(addr2reg_map.count(ir->a1));                                        \
    assert(addr2reg_map.count(ir->a2));                                        \
    ret.push_back(string("asm_op\t") + regs_name(addr2reg_map[ir->a0]) +       \
                  regs_name(addr2reg_map[ir->a1]) +                            \
                  regs_name(addr2reg_map[ir->a2]));                            \
    break;                                                                     \
  }

std::list<string>
translate(IR::Ptr ir, IR::Addr::Ptr regs[kMax_Use_Reg],
          unordered_map<IR::Addr::Ptr, int> sp_offset_map,
          unordered_map<IR::Addr::Ptr, int> addr2reg_map) {
  std::list<string> ret;
  switch (ir->op_) {
    THREE_ADDR_ONE_TO_ONE(ADD, add)
    THREE_ADDR_ONE_TO_ONE(SUB, sub)
    THREE_ADDR_ONE_TO_ONE(MUL, mul)
    THREE_ADDR_ONE_TO_ONE(DIV, sdiv)
    OP_CASE(LOAD) {}
    OP_CASE(STORE) {
    }
    // more ...
    default:
      break;
  }
  return ret;
}
#undef OP_CASE

// 拼接字符串
string concat(int num, ...) {
  string ret;
  va_list ap;
  va_start(ap, num);
  for (int i = 0; i < num; ++i) {
    char* str = va_arg(ap, char*);
    ret = ret + str;
  }
  return ret;
}

// 检查一个函数是否是叶子过程
bool check_leaf_procedure(IR::List &ir_list) {
  for (auto ir : ir_list) {
    if (ir->op_ == IR::Op::CALL) {
      return false;
    }
  }
  return true;
}

// 获取函数的最多前n个参数, 如果n为-1获取所有参数
vector<IR::Addr::Ptr> get_front_params(FunctionDecl *func, int n) {
  auto param_list = func->get_param_list();
  vector<IR::Addr::Ptr> ret;
  if (param_list) {
    if (n < 0) {
      n = param_list->size();
    }
    for (auto param : *param_list) {
      if (n >= 1) {
        ret.push_back(param->vartab_ent->addr);
        n--;
      }
    }
    return ret;
  }
  return vector<IR::Addr::Ptr>();
}
} // namespace

/**
 * pre-condition:
 *    ir_list 的第一个ir必须为FUNCDEF
 *    且最后一个必须为FUNCEND
 */
std::list<std::string> translate_function(IR::List &ir_list, FunctionDecl* func) {
  std::list<std::string> ret;
  // 是否为叶子过程
  bool leaf_procedure = check_leaf_procedure(ir_list);

  // vars 是得到的冲突图, 包含了ir_list中使用的全部变量
  auto [vars, _] = liveness_analyze(ir_list);

  // 为通过寄存器传递的参数、局部变量、
  // 函数调用时溢出的参数分配的栈空间
  int alloc_stack_size = 0;
  {
    // FIX: WARNNING: 假设变量大小为4
    const int var_size = 4;
    alloc_stack_size = var_size * std::count_if(vars.begin(), 
        vars.end(), [](IR_Addr::Ptr addr){
          return addr->kind == IR_Addr::Kind::VAR;
        });
  }

  // 现在可以确定sp的值了，为每个变量分配一个溢出时
  // 存储的相对sp偏移的栈地址
  unordered_map<IR::Addr::Ptr, int> sp_offset_map;
  {
    // FIX: WARNNING: 不应该依赖于所有变量大小都为4
    //  高地址:
    //            |-------------|
    //            | 第五个参数  |
    //            |-------------|
    //            | 第六个参数  |     父函数
    //            |-------------|
    //            |     ...     |
    // ---------------------------------------- 
    //            |  局部变量   |                   
    //            |-------------|
    //  当前视角  |    . . .    |
    //  ----->    |-------------|    <-----  sp 位置
    //            | 第五个参数  |  }
    //            |-------------|      
    //            | 第六个参数  |  }  调用子函数溢出的参数
    //            |-------------|
    //            |     ...     |  }
    // ---------------------------------------- 
    //            |  其他函数   |
    //            |-------------|
    //            |             |     子函数
    //            |-------------| 
    //  低地址:
    const int var_size = 4;
    int offset = 0 - alloc_stack_size + var_size;

    // 为局部变量分配溢出地址
    for (auto var : vars) {
      auto addr = dynamic_pointer_cast<IR_Addr>(var);
      if (addr->kind == IR_Addr::Kind::VAR){
        sp_offset_map.emplace(addr, offset);
        offset += var_size;
      }
    }

    // 将溢出参数的溢出地址与当前sp正确对应
    auto param_list = get_front_params(func, -1);
    for (int i = param_list.size() -1; i >= kMax_Use_Reg; --i) {
      sp_offset_map.emplace(param_list[i], offset);
      offset += var_size;
    }
  }

  IR::Addr::Ptr regs[kReg_Num];
  {
    for (int i = 0; i < kReg_Num; ++i) {
      regs[i] = nullptr;
    }
  }
  // 图着色
  {
    auto front_params = get_front_params(func, kTransfer_Reg);
    color_allocate alloc;
    for (auto param : front_params) {
        param->colorize(alloc.get_new_color());
    }
    colorize_num_allocate(vars, kMax_Use_Reg, alloc);
  }

  unordered_map<IR::Addr::Ptr, int> addr2ir_map;
  // 函数前四个参数包含初值
  {
    int index = 0;
    auto front_params = get_front_params(func, kTransfer_Reg);
    for (auto param : front_params) {
      addr2ir_map[param] = index;
      regs[index++] = param;
    }
  }

  /**
   *   .global <func_name>
   *   .type <func_name>, %function
   *   .thumb
   *   .thumb_func
   * <func_name>:
   */
  {
    std::string name = ir_list.front()->a0->name;
    ret.push_back(string(".global\t") + name);
    ret.push_back(string(".type\t") + name + ", %function");
    ret.push_back(string(".thumb"));
    ret.push_back(string(".thumb_func"));
    ret.push_back(name + ":");
  }
  {
    if (leaf_procedure) {
      // 叶子节点不保存lr
      ret.push_back("push\t{r4-r11}");
    }else {
      ret.push_back("push\t{lr, r4-r11}");
    }
  }
  // 分配栈空间
  {
    char buf[32];
    sprintf(buf, "#%d", alloc_stack_size);
    ret.push_back(concat(2, "sub\tsp, sp, ", buf));
  }
  // 实际翻译
  for (auto ir : ir_list) {
    ret.splice(ret.end(), translate(ir, regs, sp_offset_map, addr2ir_map));
  }

  // 回收栈空间
  {
    char buf[32];
    sprintf(buf, "#%d", alloc_stack_size);
    ret.push_back(concat(2, "add\tsp, sp, ", buf));
  }
  // 恢复栈帧
  {
    if(leaf_procedure){
      ret.push_back("pop\t{r4-r11}");
      ret.push_back("bx\tlr");
    }else {
      ret.push_back("pop\t{pc, r4-r11}");
    }
  }
  return ret;
}
