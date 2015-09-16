#ifndef SLIST_CONTEXT_H
#define SLIST_CONTEXT_H

#include <unordered_map>
#include <functional>
#include <vector>

#include "slist_types.h"

namespace slist
{
	struct context
	{
		context();

		void     register_native(const std::string& name, funcdef::callback func);
		node_ptr lookup_variable(const std::string& name);

		typedef std::unordered_map<std::string, funcdef_ptr> funcdef_map;
		funcdef_map global_funcs;

		typedef std::unordered_map<std::string, node_ptr> var_map;
		typedef std::vector<var_map> var_stack;
		var_stack variables;
	};
}

#endif