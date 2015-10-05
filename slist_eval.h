#ifndef SLIST_EVAL_H
#define SLIST_EVAL_H

#include "slist_types.h"
#include "slist_context.h"
#include <string>

namespace slist
{
	node_ptr eval(context& ctx, const node_ptr& n);
	node_ptr eval(context& ctx, const funcdef_ptr& f, const node_ptr& args);
	node_ptr apply(context& ctx, const node_ptr& args, const funcdef_ptr& proc);
	node_ptr exec(context& ctx, const std::string& str);
}

#endif