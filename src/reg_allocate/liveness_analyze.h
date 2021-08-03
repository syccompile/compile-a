#pragma once

#include "../ir.h"
// 不冲突的同在一条move语句中的两个变量
struct mov_entry{
    shared_ptr<color_node> first;
    shared_ptr<color_node> second;
    int is_coalesced;  // 表示这条move语句的合并状态 0---初始状态  1---合并状态  2---冻结状态  3---溢出状态
    int num;  // 序号
};

// 对mov_entry重载运算符!=
bool operator!=(const mov_entry &a, const mov_entry &b);
// 对mov_entry重载运算符==
bool operator==(const mov_entry &a, const mov_entry &b);

tuple<set<color_node::ptr>, vector<mov_entry>>
liveness_analyze(IR::List &ir_list);
