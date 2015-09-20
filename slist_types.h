#ifndef SLIST_TYPES_H
#define SLIST_TYPES_H

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>

#include <vector>

namespace slist
{
	struct context;

	struct node;
	typedef std::shared_ptr<node> node_ptr;

	struct funcdef;
	typedef std::shared_ptr<funcdef> funcdef_ptr;

	typedef std::unordered_map<std::string, node_ptr> var_map;
	typedef std::vector<var_map> var_stack;

	enum class node_type
	{
		empty,
		list,
		boolean,
		integer,
		number,
		string,
		symbol
	};

	struct node
	{
		node() : type(node_type::empty) { }

		bool to_bool() const;
		int to_int() const;
		float to_float() const;

		node_type type;
		std::string data;

		typedef std::vector<node_ptr> node_vector;
		node_vector children;

		// For lambdas
		funcdef_ptr proc;
	};

	struct funcdef
	{
		funcdef() : variadic(false), is_native(false) {}

		std::string name;

		typedef std::vector<std::string> arg_list;
		arg_list args;
		bool variadic;
		bool is_native;

		var_stack local_vars;

		// Body of the function (non-native)
		node_ptr body;

		// Callback (native)
		typedef std::function<node_ptr(context&, const node_ptr&)> callback;
		callback native_func;
	};

	std::string type_to_string(slist::node_type type);

	void print_node(const node_ptr& node);
	void debug_print_node(const node_ptr& node);
	void debug_print_funcdef(const funcdef_ptr& func);
}

#endif