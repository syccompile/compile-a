#include "frame.h"

int Frame::klabel_num;
static int tmp_num = 0;

std::string Frame::generateLabelName() {
  return std::string(".L") + std::to_string(klabel_num++);
}

FrameAccess Frame::newTempAccess(Frame::Ptr parent) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  access->kind_ = _FrameAccess::Kind::TEMP;
  access->name_ = std::string("tmp") + std::to_string(tmp_num++);
  access->frame_ = parent;
  return access;
}
FrameAccess Frame::newLabelAccess(Frame::Ptr parent) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  access->kind_ = _FrameAccess::Kind::LABEL;
  access->name_ = generateLabelName();
  access->frame_ = parent;
  return access;
}
FrameAccess Frame::newLabelAccess(Frame::Ptr parent, std::string label) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  access->kind_ = _FrameAccess::Kind::LABEL;
  access->name_ = label;
  access->frame_ = parent;
  return access;
}
FrameAccess Frame::newImmAccess(Frame::Ptr parent, int num) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  access->kind_ = _FrameAccess::Kind::IMM;
  access->name_ = "IMM";
  access->locate_.offset = num;
  access->frame_ = parent;
  return access;
}
FrameAccess Frame::newVarAccess(Frame::Ptr parent, std::string str) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  // FIX：假设是REG类型
  access->kind_ = _FrameAccess::Kind::REG;
  access->name_ = str;
  access->frame_ = parent;
  return access;
}

FrameAccess Frame::newRetAccess(Frame::Ptr parent) {
  FrameAccess access = std::make_shared<_FrameAccess>();
  // FIX：假设是REG类型
  access->kind_ = _FrameAccess::Kind::REG;
  access->name_ = "%ret";
  access->frame_ = parent;
  return access;
}
