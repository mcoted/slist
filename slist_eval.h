#ifndef SLIST_EVAL_H
#define SLIST_EVAL_H

#include "slist_types.h"
#include "slist_context.h"
#include <string>

namespace slist
{
	parse_node_ptr eval(context& ctx, const parse_node_ptr& tree);
	parse_node_ptr exec(context& ctx, const std::string& str);
}

#endif