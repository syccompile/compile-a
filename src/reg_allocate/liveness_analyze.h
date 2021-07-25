#pragma once

#include "../ir.h"

tuple<set<color_node::ptr>, set<color_node::ptr>>
liveness_analyze(IR::List &ir_list);
