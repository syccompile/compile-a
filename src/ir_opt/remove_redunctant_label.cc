#include <list>
#include <map>
#include "../ir.h"

void remove_redunctant_label(std::list<IR::Ptr> &l) {
  std::map <IR::Addr::Ptr, IR::Addr::Ptr> label_alias;
  std::list<IR::Ptr> new_list;

  // 前处理
  new_list.splice(new_list.end(), l, l.begin());

  // 标号消除
  while (!(l.empty())) {
    // 如果已处理列表内已经有一个标号，且已经存在一个标号序列
    if (l.front()->op_ == IR::Op::LABEL) {
      label_alias[l.front()->a0] = new_list.back()->a0;
      l.front()->a0 = new_list.back()->a0;
    }
    new_list.splice(new_list.end(), l, l.begin());
  }

  // 为被优化掉的标号替换
  for (auto i: new_list) {
    #define single_process(ADDR) \
    if (i->ADDR != nullptr && \
        i->ADDR->kind ==IR::Addr::Kind::BRANCH_LABEL && \
        label_alias.count(i->ADDR)) \
    i->ADDR = label_alias[i->ADDR];

    single_process(a0);
    single_process(a1);
    single_process(a2);

    #undef single_process
  }

  l.splice(l.end(), new_list);
}
