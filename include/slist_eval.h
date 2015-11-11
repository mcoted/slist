#ifndef SLIST_EVAL_H
#define SLIST_EVAL_H

#include "slist_types.h"
#include "slist_context.h"
#include <string>
#include <istream>

namespace slist
{
	node_ptr eval(context& ctx, const node_ptr& n);
	node_ptr eval_procedure(context& ctx, const procedure_ptr& f, const node_ptr& args);
	node_ptr apply(context& ctx, const node_ptr& args, const procedure_ptr& proc);
	node_ptr exec(context& ctx, const std::string& str);
	node_ptr exec(context& ctx, std::istream& in);
}

#endif