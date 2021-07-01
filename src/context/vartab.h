#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include "types.h"
#include "../ir.h"

// 表示单一常变量、数组常变量的信息
struct VarTabEntry {
public:
  using Ptr = std::shared_ptr<VarTabEntry>;
  using Ptr_const = std::shared_ptr<const VarTabEntry>;

  Type type;

  // 名字，存储的主要目的是方便给全局变量生成标号
  std::string name;
  // 初值，存储的主要目的是记录常量值
  std::vector<int> init_val;

  // 分配的地址（虚地址、虚拟寄存器号）
  IR::Addr::Ptr addr;
  // 是否为常量
  bool is_constant;
  // 如果该符号是参数，则为参数序号，否则为-1
  int param_order;

  VarTabEntry(std::string name, std::vector<int> shape, IR::Addr::Ptr addr, std::vector<int> init_val, bool is_const=false, int param_order=-1);
  // 判断是否为数组，规则请参考成员shape的说明
  bool is_array() const { return type.arr_shape.size()!=0; }
};

class VarTab {
private:
  std::unordered_map<std::string, VarTabEntry::Ptr> symtab;

public:
  // 添加一个变量名称
  void put(VarTabEntry::Ptr ent);
  
  // 判断符号表是否为全局表
  bool is_global() const { return fa==nullptr; }

  // 根据名称获取符号
  // 当不存在符号时，返回std::shared_ptr(nullptr)
  VarTabEntry::Ptr_const get(std::string name) const;

  std::shared_ptr<VarTab> fa;
};
