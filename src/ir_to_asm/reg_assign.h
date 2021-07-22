#pragma once
#include "../ir.h"
#include <unordered_map>

std::unordered_map<int, int>
register_assign(IR::List &l);