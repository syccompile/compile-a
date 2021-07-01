#pragma once

class Debug_impl {
public:
  virtual ~Debug_impl() { }
  virtual void internal_print() = 0;
};
