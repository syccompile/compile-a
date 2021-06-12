#pragma once
#include <string>
#include <memory>

class Frame;

/** 表示一个寄存器 **/
class Reg {};

/**
 * FrameAccess是对栈帧中对变量、标号地址、寄存器的抽象，
 * 例如如果要求栈帧分配一个变量空间，栈帧返回一个FrameAccess，
 * 后续可以通过FrameAccess来表示该变量进而生成中间代码
 *
 * 例如对于语句
 *    int a = 10*b + 3*c;
 * 首先调用 newAccess()来为变量a 分配一个FrameAccess，
 * 假设先计算10*b，再计算3*c，最后计算加法，
 * 计算10*b时，查找符号表找到b的FrameAccess，然后调用newAccess分配
 *  一个临时变量FrameAccess存储结果，得到
 *  MUL 10, b -> tmp1
 * 同理计算3*c时
 *  MUL 3, c -> tmp2
 * 最后计算加法有
 *  ADD tmp1, tmp2 -> a 
 *
 */
struct _FrameAccess {
  using Ptr = std::shared_ptr<_FrameAccess>;
  enum class Kind { MEM, REG, LABEL };

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

/**
 * 表示一个栈帧结构
 */
class Frame {
public:
  using Ptr = std::shared_ptr<Frame>;
  FrameAccess newAccess();
};
