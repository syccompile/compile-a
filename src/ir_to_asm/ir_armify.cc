#include "../ir.h"
#include "../context/context.h"
#include <unordered_map>

// 将val循环左移n位
uint32_t
rol(uint32_t val, uint32_t n) {
  n %= 32u;
  uint32_t ret;
  ret  = val << n;
  ret |= (val & (0xffffffffu << (32-n))) >> (32-n);
  return ret;
}

// 判断是否为arm立即数
bool
is_arm_imm(uint32_t imm) {
  for (int j=0 ; j<8 ; j++) {
    uint32_t rol_result = rol(imm, j*2);
    if (rol_result <= 0x000000ffu) return true;
  }
  return false;
}

// 将一个未知类型的地址a移动到ARM可接受类型的地址中
// 返回：[新地址, 需要向全局变量定义加入的IR，需要向函数定义加入的IR]
std::tuple<IR::Addr::Ptr, IR::List, IR::List>
move_into_var(IR::Addr::Ptr a) {

  // 存储无法被ARM指令表示的立即数之内存地址
  static std::unordered_map<int, IR::Addr::Ptr> imm_map;
  // 存储全局数组标号地址
  static std::unordered_map<std::string, IR::Addr::Ptr> glob_var_map;

  IR::Addr::Ptr ret = nullptr;
  IR::List ret_def, ret_func;

#define CASE(TYPE) case IR::Addr::Kind:: TYPE :

  switch(a->kind) {
    CASE(VAR)
      ret = a;
      break;
    CASE(IMM)
      // 立即数本身可
      if (is_arm_imm(a->val)) {
        ret = a;
        break;
      }
      // 立即数反值也可
      // 利用MVN
      else if (is_arm_imm(~(a->val))) {
        ret = context.allocator.allocate_addr();
        ret_func.push_back(IR::make_binary(
          IR::Op::MVN,
          ret,
          IR::Addr::make_imm(~(a->val))
        ));
      }
      // 其他立即数：必须从内存载入
      else {
        IR::Addr::Ptr imm_globl_addr = nullptr;
        ret = context.allocator.allocate_addr();

        // 先查找是否已经有该数的全局变量
        // 若无
        if (imm_map.count(a->val) == 0) {
          // 建立地址
          imm_globl_addr = IR::Addr::make_named_label(std::string(".imm_") + std::to_string(a->val));
          // 创建新全局变量
          ret_def.push_back(IR::make_unary(IR::Op::VARDEF, imm_globl_addr));
          ret_def.push_back(IR::make_unary(IR::Op::DATA, IR::Addr::make_imm(a->val)));
          ret_def.push_back(IR::make_no_operand(IR::Op::VAREND));
          // 加入表中
          imm_map[a->val] = imm_globl_addr;
        }
        // 若有
        else {
          imm_globl_addr = imm_map[a->val];
        }

        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, imm_globl_addr));
      }
      break;
    CASE(PARAM)
      // 通过寄存器传过来的参数
      if (a->val<4) {
        ret = a;
      }
      // 通过栈传过来的参数
      else {
        ret = context.allocator.allocate_addr();
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, a));
      }
      break;
    CASE(NAMED_LABEL)
      // 从全局变量表获取信息
      auto ent = context.vartab_cur->get(a->name);
      ret = context.allocator.allocate_addr();

      // a代表数组名
      // 此时移入a的地址
      if (ent->is_array()) {
        IR::Addr::Ptr label_addr = nullptr;
        // 如果这个名字的标号地址还未生成
        if (glob_var_map.count(a->name)==0) {
          label_addr = IR::Addr::make_named_label(std::string(".var_ptr_") + a->name);
          // 创建新全局变量
          ret_def.push_back(IR::make_unary(IR::Op::VARDEF, label_addr));
          ret_def.push_back(IR::make_unary(IR::Op::DATA, IR::Addr::make_imm(a->val)));
          ret_def.push_back(IR::make_no_operand(IR::Op::VAREND));
          // 加入表中
          glob_var_map[a->name] = label_addr;
        }
        // 如果这个名字的标号地址已经生成
        else {
          label_addr = glob_var_map[a->name];
        }
        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, label_addr));
      }
      // a代表单一变量名
      // 此时移入a的内容
      else {
        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, a));
      }
      break;
  }

#undef CASE

  return std::make_tuple(ret, ret_def, ret_func);

}

void
ir_armify(IR::List &defs, IR::List &funcs) {
  IR::List new_funcs;

  while (!(funcs.empty())) {
    auto ir = funcs.front();

    // ir为算术逻辑型，a0  = a1 op a2
    // 或者是cmp型     nil = a1 op a2
    if (ir->is_al() || ir->op_==IR::Op::CMP) {
      // 首先保证a1、a2在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      auto [a2, a2_def_app, a2_func_app] = move_into_var(ir->a2);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a1_def_app);
      defs.splice(defs.end(), a2_def_app);
      new_funcs.splice(funcs.end(), a1_func_app);
      new_funcs.splice(funcs.end(), a2_func_app);
      // 修改原IR
      ir->a1 = a1;
      ir->a2 = a2;
    }
    // ir为移动型
    else if (ir->is_mov()) {
      // 首先保证a1在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a1_def_app);
      new_funcs.splice(funcs.end(), a1_func_app);
      // 修改原IR
      ir->a1 = a1;
    }
  }

}
