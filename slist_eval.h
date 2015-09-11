#ifndef SLIST_EVAL_H
#define SLIST_EVAL_H

#include "slist_types.h"
#include "slist_context.h"

namespace slist
{
	node_ptr eval(context& ctx, const node_ptr& tree);
}

#endif