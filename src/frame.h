#pragma once
#include "debug.h"

#include <memory>
#include <string>

class Frame;

/** 表示一个寄存器 **/
class Reg {};

/**
 * FrameAccess是对栈帧中对变量、标号地址、寄存器、立即数的抽象，
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
 * 例如对于函数调用
 *    var = func(10, b);
 *  在处理函数声明时已经把func加入了符号表，假设func有两个形参，
 *  其FrameAccess分别为fa1, fa2, 处理上述函数调用可以如下
 *    MOV 10 -> fa1
 *    MOV b  -> fa2
 *    CALL func -> tmp
 *    MOV tmp -> var
 *  其中tmp对应函数调用结果
 *
 */
struct _FrameAccess : public Debug_impl {
  using Ptr = std::shared_ptr<_FrameAccess>;
  enum class Kind { MEM, REG, LABEL, TEMP, IMM };

  ~_FrameAccess() {}
  void copy(std::shared_ptr<_FrameAccess> that) {
    kind_ = that->kind_;
    name_ = that->name_;
    locate_ = that->locate_;
    frame_ = that->frame_;
  }
  virtual void internal_print() const override;

  Kind kind_;
  std::string name_; // Label名称，也用于debug
  union Locate {
    Locate() {}
    ~Locate() {}
    int offset; // 内存相对帧指针的偏移
    Reg reg;
  } locate_;
  /**
   * @member frame_
   * FrameAccess对应的栈帧
   */
  std::shared_ptr<Frame> frame_;
};

using FrameAccess = std::shared_ptr<_FrameAccess>;

/**
 * 表示一个栈帧结构
 */
class Frame {
public:
  using Ptr = std::shared_ptr<Frame>;
  Frame(bool global) : global_(global) {}
  ~Frame() {}
  const bool global_;
  FrameAccess newVarAccess(Frame::Ptr, std::string);
  FrameAccess newLabelAccess(Frame::Ptr);
  FrameAccess newLabelAccess(Frame::Ptr, std::string);
  FrameAccess newTempAccess(Frame::Ptr);
  FrameAccess newImmAccess(Frame::Ptr, int);
  FrameAccess newRetAccess(Frame::Ptr);

private:
  static int klabel_num;
  int tmp_num = 0;
  std::string generateLabelName();
};

extern Frame::Ptr GlobFrame;
