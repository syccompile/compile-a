#include "frame.h"

FrameAccess Frame::newTempAccess() {
  return std::make_shared<_FrameAccess>();
}
FrameAccess Frame::newLabelAccess() {
  return std::make_shared<_FrameAccess>();
}
FrameAccess Frame::newImmAccess(int num){
  return std::make_shared<_FrameAccess>();
}
FrameAccess Frame::newVarAccess() {
  return std::make_shared<_FrameAccess>();
}

