#pragma once
#include "../ir.h"
#include <string>
#include <list>
#include <utility>

std::pair<IR::List, IR::List>
split_decl_func();

std::list<IR::List>
split_functions();

std::list<std::string>
translate_function(IR::List &l);
