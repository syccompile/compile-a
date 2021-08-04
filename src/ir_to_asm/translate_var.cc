#include "translate_to_asm.h"

namespace {
  using std::list;
  using std::string;
  using std::to_string;
}

list<string>
translate_var(IR::List &l) {
  IR::List processed;
  list<string> ret;

  auto name = l.front()->a0->name;
  ret.push_back(string(".data"));
  ret.push_back(string(".global ") + name);
  ret.push_back(name + ":");
  processed.splice(processed.end(), l, l.begin());

  while (!(l.empty())) {
    auto ir = l.front();
    if (ir->op_==IR::Op::DATA) ret.push_back(string("\t.word\t") +
      ((ir->a0->kind == IR::Addr::Kind::NAMED_LABEL) ? ir->a0->name : to_string(ir->a0->val))
    );
    else if (ir->op_==IR::Op::ZERO) ret.push_back(string("\t.space\t") + to_string(4*(ir->a0->val)));
    processed.splice(processed.end(), l, l.begin());
  }

  l.splice(l.end(), processed);
  return ret;
}
