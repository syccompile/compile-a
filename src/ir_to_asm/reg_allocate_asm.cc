#include "translate_to_asm.h"
#include "../reg_allocate/liveness_analyze.h"

#include <stdarg.h>
#include <unordered_map>
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
namespace {
// 总共有15个寄存器
const int kReg_Num = 15;

// 最多可以使用r0-r11的寄存器
const int kMax_Use_Reg = 11;

// 前4个参数使用寄存器传递
const int kTransfer_Reg = 4;

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

// 获取regs中一个空寄存器或者溢出变量的寄存器
int regs_find_null(IR::Addr::Ptr regs[kMax_Use_Reg]) {
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
// TODO
int chose_a_reg(IR::Addr::Ptr regs[kMax_Use_Reg]) { return 0; }

// pre-condition: addr不能是溢出的
std::list<string>
load_addr_to_reg(IR::Addr::Ptr addr, IR::Addr::Ptr regs[kMax_Use_Reg],
                 const unordered_map<IR::Addr::Ptr, int> sp_offset_map,
                 unordered_map<IR::Addr::Ptr, int> addr2reg_map) {
  std::list<string> ret;
  if (addr->get_color() == spill_color()) {
    // 该变量是溢出的
    int null_reg = regs_find_null(regs);
    if (null_reg >= 0) {
      {
        // 如果存在空的寄存器，加载到空寄存器上
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

      // 将该寄存器上相同颜色的变量写回栈
      int reg_num = chose_a_reg(regs);
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
      {
        // 加载新的寄存器值
        regs[reg_num] = addr;
        int offset = sp_offset_map.at(addr);
        ret.emplace_back(string("ldr\t") + regs_name(reg_num) + ", [sp], #" +
                         to_string(offset));
        addr2reg_map[addr] = reg_num;
      }
    }
  } else if (addr2reg_map.count(addr)) {
    // addr已经在reg中
  } else {
    int reg_num = regs_find_same_color(regs, addr->get_color());
    bool find_same_color = reg_num >= 0;
    if (!find_same_color) {
      reg_num = regs_find_null(regs);
      // 不可能找不到一个空的寄存器因为addr是着色节点
      assert(reg_num >= 0);
    }
    // 如果找到一个溢出变量的寄存器，写回
    if (!find_same_color && regs[reg_num]) {
        int offset = sp_offset_map.at(regs[reg_num]);
        ret.emplace_back(string("str\t") + regs_name(reg_num) + ", [sp], #" +
                         to_string(offset));
        addr2reg_map.erase(regs[reg_num]);
    }
    // 如果得到的寄存器addr与该addr颜色相同，表明它们共用一个
    // 寄存器，不用将其保存到栈中
    {
      // 加载新的寄存器值
      regs[reg_num] = addr;
      int offset  =sp_offset_map.at(addr);
      ret.emplace_back(string("ldr\t") + regs_name(reg_num) + ", [sp], #" + to_string(offset));
      addr2reg_map[addr] = reg_num;
    }
  }
  return ret;
}

#define OP_CASE(op)  case IR::Op::op:
std::list<string>
translate(IR::Ptr ir, IR::Addr::Ptr regs[kMax_Use_Reg],
          unordered_map<IR::Addr::Ptr, int> sp_offset_map,
          unordered_map<IR::Addr::Ptr, int> addr2reg_map) {
  std::list<string> ret;
  switch (ir->op_) {
    OP_CASE(ADD) {
      ret.splice(ret.end(), load_addr_to_reg(ir->a1, regs, sp_offset_map, addr2reg_map));
      ret.splice(ret.end(), load_addr_to_reg(ir->a2, regs, sp_offset_map, addr2reg_map));
      ret.splice(ret.end(), load_addr_to_reg(ir->a0, regs, sp_offset_map, addr2reg_map));
      // 保证三个变量都在寄存器中
      assert(addr2reg_map.count(ir->a0));
      assert(addr2reg_map.count(ir->a1));
      assert(addr2reg_map.count(ir->a2));
      ret.push_back(string("add\t") + 
          regs_name(addr2reg_map[ir->a0]));
      break;
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
  // TODO
  return false;
}

// 获取函数的最多前n个参数, 如果n为-1获取所有参数
vector<IR::Addr::Ptr> get_front_params(IR::List &ir_list, int n) {
  return vector<IR::Addr::Ptr>();
}
} // namespace

/**
 * pre-condition:
 *    ir_list 的第一个ir必须为FUNCDEF
 *    且最后一个必须为FUNCEND
 */
std::list<std::string> translate_function(IR::List &ir_list) {
  std::list<std::string> ret;
  // 是否为叶子过程
  bool leaf_procedure = check_leaf_procedure(ir_list);

  // 为通过寄存器传递的参数、局部变量、
  // 函数调用时溢出的参数分配的栈空间
  int alloc_stack_size = 0;
  // TODO: calc size

  // vars 是得到的冲突图
  auto [vars, _] = liveness_analyze(ir_list);

  // 现在可以确定sp的值了，为每个变量分配一个溢出时
  // 存储的相对sp偏移的栈地址
  unordered_map<IR::Addr::Ptr, int> sp_offset_map;
  {
    // WARNNING: 不应该依赖于所有变量大小都为4
    const int var_size = 4;
    int offset = 0 - alloc_stack_size + var_size;
    for (auto param : get_front_params(ir_list,  -1)) {
      sp_offset_map.emplace(param, offset);
      offset += var_size;
    }

    for(auto var : vars){
      sp_offset_map.emplace(dynamic_pointer_cast<IR_Addr>(var), offset);
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
    auto front_params = get_front_params(ir_list, kTransfer_Reg);
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
    auto front_params = get_front_params(ir_list, kTransfer_Reg);
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
    if(leaf_procedure){
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
