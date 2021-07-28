#pragma once

#include "../ir.h"

tuple<set<color_node::ptr>, set<pair<color_node::ptr, color_node::ptr> > >
liveness_analyze(IR::List &ir_list);
