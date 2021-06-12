#pragma once
#include <string>
#include <memory>

// 前置声明
class Frame;

class Reg {};

struct _FrameAccess {
  enum class Kind { MEM, REG, LABEL };

  _FrameAccess(std::shared_ptr<Frame> frame) : frame_(frame) {}
  ~_FrameAccess() {}

  Kind kind_;
  union Locate{
    Locate() {}
    ~Locate() {}
    int offset;
    Reg reg;
    std::string name;
  } locate_;
  std::shared_ptr<Frame> frame_;
};

using FrameAccess = std::shared_ptr<_FrameAccess>;

class Frame {
public:
  using Ptr = std::shared_ptr<Frame>;
};
