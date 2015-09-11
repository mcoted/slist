#ifndef SLIST_CONTEXT_H
#define SLIST_CONTEXT_H

#include <unordered_map>
#include <functional>

#include "slist_types.h"

namespace slist
{
	typedef std::function<node_ptr(const node_ptr& node)> func;

	struct context
	{
		context();

		std::unordered_map<std::string, func> global_funcs;
	};
}

#endif