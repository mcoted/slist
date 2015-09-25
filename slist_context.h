#ifndef SLIST_CONTEXT_H
#define SLIST_CONTEXT_H

#include <vector>

#include "slist_types.h"

namespace slist
{
	struct context
	{
		context();

		void  register_native(const std::string& name, funcdef::callback func);
		node_ptr lookup_variable(const std::string& name);

		var_stack global_vars;
	};
}

#endif