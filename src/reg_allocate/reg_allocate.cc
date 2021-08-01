#include "liveness_analyze.h"
#include "../context/context.h"
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

/** pre-condition:
 *  ir_list 的第一个ir必须为FUNCDEF
 *  且最后一个必须为FUNCEND
 */
void
register_allocate(IR::List &ir_list) {
  // vars 是得到的冲突图
  auto [vars, mov_related] = liveness_analyze(ir_list);

  // 获取函数信息
  string funcname = ir_list.front()->a0->name;
  auto   functab_ent = context.functab->get(funcname);

  // 预着色
  color_allocate alloc;
  int i = 0;
  for (auto param : functab_ent->param_list) {
    param->colorize(alloc.get_new_color());
    i++;
    if (i>=4) break;
  }

  // 处理MOV
  for (auto &i: mov_related) {
    process_mov(i, alloc);
  }

  // 图着色
  colorize_nodes_allocate(vars, alloc);
}
