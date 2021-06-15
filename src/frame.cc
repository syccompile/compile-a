#include "frame.h"


int Frame::klabel_num;

std::string Frame::generateLabelName() {
  return std::string(".L") + std::to_string(klabel_num++);
}

FrameAccess Frame::newTempAccess(Frame::Ptr parent) { return std::make_shared<_FrameAccess>(); }
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
  return std::make_shared<_FrameAccess>();
}
FrameAccess Frame::newVarAccess(Frame::Ptr parent) { return std::make_shared<_FrameAccess>(); }

FrameAccess Frame::newRetAccess(Frame::Ptr parent) { return std::make_shared<_FrameAccess>(); }
