#include "types.h"

Type::Type(std::vector<int> &&shape): arr_shape(shape), basic_type(BasicType::INT) { }