#include "liveness_analyze.h"
#include "../context/context.h"
#include <stdarg.h>
#include <unordered_map>
#include <assert.h>
#include <map>

// 改进图着色设计说明
// 基本按照虎书思路进行，不过沿用了“不考虑溢出”的一些思想，方便和后面对接
//  因为预留了两个调度寄存器，所以对于“真实溢出变量”，直接给他分配一种颜色就行，后续根据溢出优先级，选择合适的溢出
// 改进图着色能达到的预期效果是：尽可能合并多的mov指令中的寄存器，以尽可能减少颜色（寄存器）的使用
// 关于为什么能减少颜色（寄存器）的使用
//  前一个版本的颜色分配，是在先合并mov后，按照vars中的变量顺序和冲突图，分配颜色
//  如果一条mov中的两个变量 1 和 2， 本可以在化简后合并
//  但不化简无法合并，受分配顺序的影响，1和2又被分配了不同颜色，
//  那么和1，2同时冲突的变量3在分配颜色时，可分配给他的颜色的数目，会因此减少，可能导致使用的总颜色数增加

using std::string;
using std::unordered_map;

// 函数中使用的所有变量
set<color_node::ptr> vars_;
// 不冲突的同在一条move语句中的两个变量集
vector<mov_entry> mov_related_;
// mov_related_的初始值拷贝，在给合并结点涂色时要用（因为合并会改变mov_related_）
vector<mov_entry> mov_related_copy;
// 预着色变量的num
vector<int> precolor_nums;
// 全体变量的num
vector<int> all_nums;
// 简化栈，存储被简化的变量
set<color_node::ptr> simplify_list;
// 是否进行合并标志变量
int flag_coal;
// 是否进行冻结标志变量
int flag_freeze;
// 可用寄存器数目
int K = 10;

map<int, int> num_color_list;

// 遍历vars  给所有变量（VAR + PARAM）标号，即给num赋值
// 同时给变量的degree赋初值 = neighbors.size()
// 同时给is_del赋初值 = flase
// 预着色，给precolor_nums赋初值
// 根据vars_.size(),给all_nums赋初值
color_allocate initialize(IR::List &ir_list){
  // 由于可能有函数调用，要对这些全局变量清零
  precolor_nums.clear();
  all_nums.clear();
  simplify_list.clear();
  num_color_list.clear();
  // 初始化num,degree,is_del,is_move
  int num = 1;
  for(auto it : vars_){
    auto it_var = dynamic_pointer_cast<var>(it);
    all_nums.push_back(num);
    it_var->num = num++;
    it_var->degree = it_var->neighbors.size();
    it_var->is_del = false;
    it_var->is_move = false;
  }
  // 初始化is_move
  for(auto it_mov : mov_related_){
    auto first = dynamic_pointer_cast<var>(it_mov.first);
    auto second = dynamic_pointer_cast<var>(it_mov.second);
    first->is_move = true;
    second->is_move = true;
  }

  // 获取函数信息
  string funcname = ir_list.front()->a0->name;
  auto   functab_ent = context.functab->get(funcname);

  // 预着色
  color_allocate alloc;
  int i = 0;
  for (auto param : functab_ent->param_list) {
    param->colorize(alloc.get_new_color());
    auto var_param = dynamic_pointer_cast<var>(param);
    // 将预着色变量num加入precolor_nums
    precolor_nums.push_back(var_param->num);
    // 将(已着色num, color)加入num_color_list
    num_color_list.insert(pair<int, int> (var_param->num, param->get_color()));
    i++;
    if (i>=4) break;
  }
  return alloc;
}

// Geogre MOV节点合并判断
// 如果对a的每一个邻居t，或者t与b冲突，或者t是低度数结点（度<K），那么a和b可以合并
bool Geogre_mov_(color_node::ptr fir, color_node::ptr sec) {
    auto fir_ = std::dynamic_pointer_cast<var>(fir);
    auto sec_ = std::dynamic_pointer_cast<var>(sec);
    int flag = 0;
    // 遍历fir的每一个邻居
    for(auto neighbor : fir_->neighbors){
        vector<shared_ptr<var>>::iterator it;
        it = find(sec_->neighbors.begin(), sec_->neighbors.end(), neighbor);
        if(neighbor->neighbors.size() < 10 || it != sec_->neighbors.end()){
            flag = 1;
            break;
        }
    }
    return flag == 1;
}

// 简化操作
// 遍历vars_,找到度数<K的mov无关结点，从vars_中删除(is_del = true)，压入simplify_list
// 删除后，更新dele邻接的结点的degree
void
simplify(){
  int simplify_en;
  do{
    simplify_en = 0;
    for(auto it : vars_){
      auto var_it = dynamic_pointer_cast<var>(it);
      // 不是预着色结点、没被删除、不是mov相关结点、度小于K
      if(it->is_colored()==false && var_it->is_del==false && var_it->is_move==false && var_it->degree < K){
        simplify_en = 1;
        var_it->is_del = true;
        all_nums.erase(remove(all_nums.begin(), all_nums.end(), var_it->num), all_nums.end());
        simplify_list.insert(it);
        // 删除结点的邻居度数-1
        for(auto neighbor : var_it->neighbors){
          if(neighbor->is_del==false)
            neighbor->degree--;
        }
      }
    }
  }while(simplify_en==1);
}


//两个vector求并集
vector<shared_ptr<var>>
vectors_set_union(vector<shared_ptr<var>> v1,vector<shared_ptr<var>> v2) {
    vector<shared_ptr<var>> v;
    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());
    set_union(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(v));//求交集
    return v;
}

// 合并操作
// 合并要做的操作就是，把其中一个删除，然后把删除的那个变量的所有出现，变成没删除的那个，更新mov_related_
//  ① 把keep和dele的num统一，dele: is_del = true
//  ② 在mov_list_中把dele替换为keep
//  ③ 遍历mov_list_，看keep是否仍为move相关变量，对is_move更新
void
coalesced(){
  int coalesced_en;
  do{
    coalesced_en = 0;
    for(auto it = mov_related_.begin(); it != mov_related_.end(); it++){
      bool fir = it->first->is_colored();
      bool sec = it->second->is_colored();
      bool both = fir && sec;  // 不能同时都是预着色结点
      if(!both && it->is_coalesced == 0 && (Geogre_mov_(it->first, it->second) || Geogre_mov_(it->second, it->first))){
        auto first = dynamic_pointer_cast<var>(it->first);
        auto second = dynamic_pointer_cast<var>(it->second);
        shared_ptr<var> dele = sec? first : second;             // 要删除的结点（不能是预着色结点）
        shared_ptr<var> keep = (dele==second)? first : second;  // 要留下的结点
        // dele标志为删除，更新all_nums
        dele->is_del = true;
        all_nums.erase(remove(all_nums.begin(), all_nums.end(), dele->num), all_nums.end());
        // dele的num更新为keep->num
        dele->num = keep->num;
        // move语句合并标志
        it->is_coalesced = 1;
        // 本回合（大范围）发生了合并标志置1--flag_coal是全局变量
        flag_coal=1;
        // 本次（小范围）发生了合并标志置1
        coalesced_en = 1;

        // 把dele的所有邻居都转给keep---求并集
        vector<shared_ptr<var>> my_union = vectors_set_union(keep->neighbors, dele->neighbors);
        keep->neighbors = my_union;
        // 不能根据my_union的大小来确定keep的度数，要根据My_union结点是否被删除
        int degree = 0;
        for(auto it_union : my_union){
          if(it_union->is_del == false){
            degree++;
          }
        }
        keep->degree = degree;

        // 更新dele的所有邻居的邻居，  如果这些邻居包含keep，度数-1，如果不包含，就增加keep。度数不变
        for(auto it_neighbor : dele->neighbors){
          vector<shared_ptr<var>>::iterator iter = find(it_neighbor->neighbors.begin(), it_neighbor->neighbors.end(), keep);
          if(iter == it_neighbor->neighbors.end()){
            it_neighbor->neighbors.push_back(keep);
          }
          else{
            it_neighbor->degree--;
          }
        }

        // 更新mov_related_
        auto father_keep = dynamic_pointer_cast<color_node>(keep);
        auto father_dele = dynamic_pointer_cast<color_node>(dele);
        for(auto it_update = mov_related_.begin(); it_update != mov_related_.end(); it_update++){
          if(it != it_update){
            if(it_update->first == father_dele){
              it_update->first = father_keep;
            }
            else if(it_update->second == father_dele){
              it_update->second = father_keep;
            }
          }
        }

        // 判断keep是否仍mov相关
        int flag = 0;
        for(auto it_mov : mov_related_){
          if(it_mov.is_coalesced == 0 && (it_mov.first==keep || it_mov.second==keep)){
            flag = 1;
            break;
          }
        }
        if(flag == 0){
          keep->is_move = false;
        }
        // 为什么不直接删除结点呢？  主要是考虑simplify掉的变量
        // 这些被合并掉的变量的颜色，就等于合并后的变量的颜色，有所托付
        // 但是，simplify掉的变量如果删除，就没有了邻接关系，着色时就没有依据
      }
    }
  }while(coalesced_en);
}

// 冻结操作
// 把一个degree < K 的move相关变量的 mov 改成false即可
void
freeze(){
  for(auto it : vars_){
    auto var_it = dynamic_pointer_cast<var>(it);
    if(var_it->is_del==false && var_it->degree < K && var_it->is_move == true){
      flag_freeze = 1;
      var_it->is_move = false;
      // 该mov变量被冻结后，mov_related中含该变量的语句都失效
      for(auto it_mov = mov_related_.begin(); it_mov != mov_related_.end(); it_mov++){
        if(it_mov->is_coalesced==0 && (it_mov->first==var_it || it_mov->second==var_it)){
          it_mov->is_coalesced = 2;
        }
      }
      break;
    }
  }
}

// 溢出操作
// 随机（优先选非mov变量）选一个度数 >= K 的结点，删除，压入simplify_list
// (更新mov_related)
void
spill(){
  // 能走到这一步，说明剩下的都是高度数结点
  bool del_mov = true;
  // 删除非mov相关的高度数结点，操作类似simplify
  for(auto it : vars_){
    auto var_it = dynamic_pointer_cast<var>(it);
    if(var_it->is_del==false && var_it->is_move==false){
      var_it->is_del = true;
      simplify_list.insert(it);
      all_nums.erase(remove(all_nums.begin(), all_nums.end(), var_it->num), all_nums.end());
      for(auto neighbor : var_it->neighbors){
        if(neighbor->is_del==false)
          neighbor->degree--;
      }
      del_mov = false;
      break;
    }
  }

  // 删除mov相关高度数结点，走到这一步说明，省的全都是mov相关高度数结点
  if(del_mov==true){
    for(auto it : vars_){
      auto var_it = dynamic_pointer_cast<var>(it);
      // 找到的第一个存在的结点，给它删了
      if(var_it->is_del==false){
        var_it->is_del = true;
        simplify_list.insert(it);
        //删除的是mov相关变量，那么在mov_related_表中的含该变量的mov_entry肯定不能再合并了
        for(auto it : mov_related_){
          if(it.is_coalesced==0 && (it.first==var_it || it.second==var_it)){
            it.is_coalesced = 3;  // 表示删除状态
            // 如果和该删除变量同属于一条mov语句,且仅属于这条语句，那么它也mov无关
            shared_ptr<color_node> spill_mov_neighbor = (it.first==var_it)? it.second : it.first;
            int i=0;     // 根据i的值判断 spill_mov_neighbor是否还mov相关
            for(auto it_ : mov_related_){
              if(it!=it_ && it_.is_coalesced==0 && (spill_mov_neighbor==it_.first || spill_mov_neighbor==it_.second)){
                break;
              }
              i++;
            }
            if(i == mov_related_.size()){
              auto mov_neighbor = dynamic_pointer_cast<var>(spill_mov_neighbor);
              mov_neighbor->is_move = false;
            }
          }
        }
        break;
      }
    }
  }
}

//结束标志：未被删除的变量都是预着色结点
bool
is_end(){
  if(all_nums == precolor_nums){
    return true;
  }
  return false;
}

/** pre-condition:
 *  ir_list 的第一个ir必须为FUNCDEF
 *  且最后一个必须为FUNCEND
 */
void
register_allocate(IR::List &ir_list) {
  // vars 是得到的冲突图
  auto [vars, mov_related] = liveness_analyze(ir_list);
  // 将vars, mov_related拷贝为全局变量
  vars_ = vars;
  mov_related_ = mov_related;
  // mov_related_copy保留mov_related的初始值
  mov_related_copy = mov_related;
  // 初始化，返回alloc，在后面继续用alloc着色
  color_allocate alloc = initialize(ir_list);
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
  // 这部分工作的目的，是为了合并更多的mov语句，一点程度减少寄存器使用
  colorize_nodes_allocate(simplify_list, alloc,num_color_list);

  // 遍历mov_related_cpoy  给合并的结点着色
  auto it = mov_related_.begin();
  for(auto it_copy=mov_related_copy.begin(); it_copy != mov_related_copy.end(); it_copy++){
    // 将是否合并的信息传递给mov_related_copy
    it_copy->is_coalesced = it->is_coalesced;
    if(it->is_coalesced==1){
      if(it->first->is_colored()){
        it->second->colorize(it->first->get_color());
      }
      else if(it->second->is_colored()){
        it->first->colorize(it->second->get_color());
      }
    }
    it++;
  }
}
