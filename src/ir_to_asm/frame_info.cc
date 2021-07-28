#include "frame_info.h"
#include <unordered_map>

FrameInfo::FrameInfo()
  : param_size(0), local_arr_size(0), spill_size(0), register_save_size(0), reg_alloc() { }

int
FrameInfo::spill_offset() const {
  return param_size + local_arr_size;
}

int
FrameInfo::local_arr_offset() const {
  return param_size;
}

int
FrameInfo::param_offset() const {
  return 0;
}

int
FrameInfo::frame_offset() const {
  return param_size + local_arr_size + spill_size + register_save_size;
}
