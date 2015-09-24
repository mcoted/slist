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

	struct node;
	typedef std::shared_ptr<node> node_ptr;	

	struct funcdef;
	typedef std::shared_ptr<funcdef> funcdef_ptr;

	typedef std::unordered_map<std::string, parse_node_ptr> var_map;
	typedef std::vector<var_map> var_stack;

	typedef std::unordered_map<std::string, node_ptr> var_map2;
	typedef std::vector<var_map2> var_stack2;


	enum class node_type
	{
		empty,
		list,
		pair,
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

		// For lambdas
		funcdef_ptr proc;
	};

	struct node : public std::enable_shared_from_this<node>
	{
		size_t length() const;
		node_ptr get(size_t index);
		void append(const node_ptr& n);

		bool to_bool() const;
		int to_int() const;
		float to_float() const;

		node_type type;
		std::string value;

		node_ptr car;
		node_ptr cdr;

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

	void print_node(const node_ptr& n);
	void print_parse_node(const parse_node_ptr& parse_node);
	void debug_print_parse_node(const parse_node_ptr& parse_node);
	void debug_print_funcdef(const funcdef_ptr& func);

	void debug_print_node(const node_ptr& n, int indent = 0);
}

#endif