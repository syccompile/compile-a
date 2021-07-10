#include <list>
#include <map>
#include "../ir.h"

void remove_redunctant_label(std::list<IR::Ptr> &l) {
  /*
  std::map<int, int> label_alias;
  std::list<IR::Ptr> new_list;
  int alias_acc = -1;

  new_list.splice(new_list.end(), l, l.begin());
  if (new_list.back()->op_ == IR::Op::LABEL)
  label_alias[new_list.back()->a0->val] = ++alias_acc;

  while (!(l.empty())) {
    // 如果处理到一个标号
    if (l.front()->op_ == IR::Op::LABEL) {
      // 且已经存在了一个连续标号序列
      if (new_list.back()->op_ == IR::Op::LABEL) {
        label_alias[l.front()->a0->val] = alias_acc;
        l.pop_front();
      }
      // 还不存在一个连续标号序列
      else {
        new_list.splice(new_list.end(), l, l.begin());
        if (new_list.back()->op_ == IR::Op::LABEL)
  label_alias[new_list.back()->a0->val] = ++alias_acc;
      }
    }
    // 没处理到标号
    else {
      new_list.splice(new_list.end(), l, l.begin());
    }
  }

  for (auto i: new_list) {
    #define single_process(ADDR) if (i->ADDR != nullptr && i->ADDR->kind ==
  IR::Addr::Kind::BRANCH_LABEL)\ i->ADDR =
  IR::Addr::make_label(label_alias[i->ADDR->val]);

    single_process(a0);
    single_process(a1);
    single_process(a2);

    #undef single_process
  }

  l.splice(l.end(), new_list);
  */
}
