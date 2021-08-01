#include "liveness_analyze.h"
#include "../context/context.h"
#include <stdarg.h>
#include <unordered_map>
#include <assert.h>

// 改进图着色设计说明
// 基本按照虎书思路进行，不过沿用了“不考虑溢出”的一些思想
// 因为预留了两个调度寄存器，所以对于“真实溢出变量”，直接给他分配一种颜色就行，后续根据溢出优先级，选择合适的溢出
// 改进图着色能达到的预期效果是：尽可能合并多的mov指令中的寄存器，以尽可能减少颜色（寄存器）的使用
// 关于为什么能减少颜色（寄存器）的使用，说一点个人理解
// 前一个版本的颜色分配，是在先合并mov后，按照vars中的变量顺序和冲突图，分配颜色
// 如果一条mov中的两个变量 1 和 2， 本可以在化简后合并
// 但不化简无法合并，受分配顺序的影响，1和2又被分配了不同颜色，
// 那么和1，2同时冲突的变量3在分配颜色时，可分配给他的颜色的数目，会因此减少，可能导致使用的总颜色数增加

using std::string;
using std::unordered_map;

// 函数中使用的所有变量
set<color_node::ptr> vars_;
// 变量中函数的参数
set<pair<color_node::ptr, color_node::ptr> > mov_related_;
// 预着色变量num
vector<int> precolor_nums;
// 全体num
vector<int> all_nums;
// 简化栈
set<color_node::ptr> simplify_list;
// 是否进行合并标志变量
int flag_coal;
// 是否进行冻结标志变量
int flag_freeze;

// 遍历vars  给所有变量（VAR + PARAM）标号，即给num赋值
// 同时给变量的degree赋初值 = neighbors.size()
// 同时给is_del赋初值 = flase
// 预着色，给precolor_nums赋初值
// 根据vars_.size(),给all.nums赋初值
void initialize(IR::List &ir_list){
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
    //...
}

// 简化操作
// 遍历vars_,找到度数<K的mov无关结点，从vars_中删除(is_del = true)，压入simplify_list
// 删除后，更新每个与删除结点邻接的结点的degree
void
simplify(){

}

// 合并操作
// 合并要做的操作就是，把其中一个删除，然后把删除的那个变量的所有出现，变成没删除的那个
//    在这里，如果两变量可以合并，先把两个num统一，然后把被合并变量的邻居的degree-1，is_del = true
// 统一之后，判断合并后的变量是否mov相关，更新mov的值
//    两个结点合并之后，mov_related表肯定会删除一个偶对，如果这两个点没有传送有关的节点了，那就意味着他们可以被简化，把mov改为false即可
void
coalesced(){

}

// 冻结操作
// 把一个degree < K 的move相关变量的 mov 改成false即可
void
freeze(){

}

// 溢出操作
// 随机（优先选非mov变量）选一个度数 > K 的结点，删除，压入simplify_list
// (更新mov_related)
void
spill(){

}

//结束标志：未被删除的变量都是预着色结点
bool
is_end(){

}

/** pre-condition:
 *  ir_list 的第一个ir必须为FUNCDEF
 *  且最后一个必须为FUNCEND
 */
void
register_allocate(IR::List &ir_list) {
  // vars 是得到的冲突图
  auto [vars, mov_related] = liveness_analyze(ir_list);
  // 将vars mov_related拷贝为全局变量
  vars_ = vars;
  mov_related_ = mov_related;
  // 所有的简化、合并等操作，都不会改变邻接关系等等信息，只会改变我定义的几个标示变量(num  degree  is_del)的值
  initialize(ir_list);
  do{
    simplify();
    // 如果简化之后，没结束，那就去合并
    if(!is_end()){
      flag_coal=0;
      coalesced();
      if(!flag_coal){// 如果没发生合并，那就去冻结
        flag_freeze = 0;
        freeze();
        if(!flag_freeze){
          // 如果没发生冻结，那就溢出
          spill();
        }
      }
    }
  }while(!is_end());


  // 图着色：变量vars_, 根据num着色
  // 最开始完整的冲突图还在，上面的操作只是改变了一些标识变量
  // 在着色时，根据simplify_list中每个变量的num，“无溢出概念”地分配颜色
  // 即，即便是真实溢出，也给他着色，因为有了备用寄存器的处理方案，加上之后的溢出优先级计算方法，一定是可以着色的
  // 这部分工作的目的，是为了合并更多的mov语句，一点程度减少寄存器使用（这个方法可以保证虚拟着色阶段，尽量用最少数量的num）
  color_allocate alloc;
  colorize_nodes_allocate(vars, alloc);
}
