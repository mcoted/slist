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

	struct parse_node;
	typedef std::shared_ptr<parse_node> parse_node_ptr;

	struct eval_node;
	typedef std::shared_ptr<eval_node> eval_node_ptr;

	struct funcdef;
	typedef std::shared_ptr<funcdef> funcdef_ptr;

	typedef std::unordered_map<std::string, parse_node_ptr> var_map;
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

	struct parse_node
	{
		parse_node() : type(node_type::empty) { }

		bool to_bool() const;
		int to_int() const;
		float to_float() const;

		node_type type;
		std::string data;

		typedef std::vector<parse_node_ptr> parse_node_vector;
		parse_node_vector children;

		parse_node_ptr next;

		// For lambdas
		funcdef_ptr proc;
	};

	struct eval_node
	{
		node_type type;
		std::string data;
		eval_node_ptr next;
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
		parse_node_ptr body;

		// Callback (native)
		typedef std::function<parse_node_ptr(context&, const parse_node_ptr&)> callback;
		callback native_func;
	};

	std::string type_to_string(slist::node_type type);

	void print_parse_node(const parse_node_ptr& parse_node);
	void debug_print_parse_node(const parse_node_ptr& parse_node);
	void debug_print_funcdef(const funcdef_ptr& func);
}

#endif