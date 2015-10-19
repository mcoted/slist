#ifndef SLIST_CONTEXT_H
#define SLIST_CONTEXT_H

#include <unordered_set>

#include "slist_types.h"

namespace slist
{
	struct context
	{
		context();

		void     register_native(const std::string& name, funcdef::callback func);
		node_ptr lookup_symbol(const std::string& name);
		void     insert_symbol(const node_ptr& node);

		environment_ptr global_env;
		environment_ptr active_env;

		typedef std::unordered_map<std::string, node_ptr> symbols_map;
		symbols_map symbols;
	};
}

#endif