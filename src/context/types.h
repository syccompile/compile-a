#pragma once
#include <vector>

class Type {
public:
  enum BasicType: unsigned {
    INT,
    VOID,
    UNKNOWN
  };

  // 基本类型
  BasicType basic_type;

  // 数组形状，如果空表示单一变量
  // 如果不空，则表示数组形状，越接近数组起始位置的数字表示越大的单元
  // 例如：{2,3,4}表示 array[2][3][4]
  //      即：a 2-sized array of (a 3-sized array of (a 4-sized array of (<type>)))
  std::vector<int> arr_shape;

  Type() = default;
  Type(std::vector<int> &&shape);

  bool is_array() const;
  bool is_param() const;
  bool get_size() const;
};
