#include "liveness_analyze.h"

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
  auto [vars, params] = liveness_analyze(ir_list);

  // 预着色
  color_allocate alloc;
  for (auto param : params) {
    param->colorize(alloc.get_new_color());
  }

  // 图着色
  colorize_nodes_allocate(vars, alloc);
}
