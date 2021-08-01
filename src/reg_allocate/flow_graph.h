#pragma once
#include <memory>
#include <string>
#include <set>
#include <algorithm>

#include "color_graph.h"
using namespace color_graph;
using namespace std;
// 代表一个变量 
class var : public color_node{
public :
  //var(string n) : name(n) {}
  var(): color_node() { }

  // 连接两个变量，表示两变量冲突
  static void link(shared_ptr<var> lhs, shared_ptr<var> rhs) {
    if(!lhs->is_neighbored(rhs) && lhs != rhs){
      lhs->neighbors.push_back(rhs);
      rhs->neighbors.push_back(lhs);
    }
  }

  // 取消连接两个变量
  static void unlink(shared_ptr<var> lhs, shared_ptr<var> rhs) {
    if(lhs->is_neighbored(rhs) && lhs != rhs){
      lhs->neighbors.erase(find(lhs->neighbors.begin(), lhs->neighbors.end(), rhs));
      rhs->neighbors.erase(find(rhs->neighbors.begin(), rhs->neighbors.end(), lhs));
    }
  }

  virtual bool is_neighbored(ptr p) override {
    return find(neighbors.begin(), neighbors.end(), p) != neighbors.end();
  }

  virtual nodes get_neighbors() override {
    nodes n;
    for (auto p : neighbors) {
      n.insert(p);
    }
    return n;
  }

  virtual ~var() {}
  
  // 存储冲突的变量
  vector<shared_ptr<var>> neighbors;
  int num;   // 变量标号
  int degree; // 代表变量的度
  bool is_del; // 是否被删除标志
  // 变量名称
  // string name;
};

// 代表变量使用操作，可以是一个ir节点或一个语句
// 作为流图中的节点
// 例如mov a -> b, b = a + 1, 都抽象成使用a的值生成b的值
class varUse {
public:
  using ptr = shared_ptr<varUse>;
  // 操作右边使用的变量
  set<shared_ptr<var>> used;
  // 操作左边的变量，也就是发生变化的变量
  set<shared_ptr<var>> def;
  // 节点的前驱节点
  vector<ptr> pred;
  // 节点的后继节点
  vector<ptr> succ;

  // 一个结点对某个变量是入口活跃的，表明该节点_或_后继节点使用了该变量的当前值
  set<shared_ptr<var>> in;
  // 一个结点对某个变量是出口活跃的，表明该节点_的_后继节点使用了该变量的当前值
  set<shared_ptr<var>> out;

  // 将某个变量从used和def中替换
  void replace_var(shared_ptr<var> old_var, shared_ptr<var> new_var) {
    if(used.erase(old_var)){
      used.insert(new_var);
    }
    if(def.erase(old_var)){
      def.insert(new_var);
    }
  }

  // 标记一个指令是否为传送指令
  bool is_move;

  // for debug
  string arrow_output;

  virtual ~varUse() {}
};
