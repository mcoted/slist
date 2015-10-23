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

	struct environment;
	typedef std::shared_ptr<environment> environment_ptr;

	enum class node_type
	{
		empty,
		pair,
		boolean,
		integer,
		number,
		name,
		string,
	};

	struct node : public std::enable_shared_from_this<node>
	{
		size_t length() const;
		node_ptr get(size_t index);
		void append(const node_ptr& n);
		node_ptr pop();

		bool to_bool() const;
		int to_int() const;
		float to_float() const;

		node_type type;
		std::string value;

		node_ptr car;
		node_ptr cdr;

		funcdef_ptr proc;
	};

	struct funcdef : public std::enable_shared_from_this<funcdef>
	{
		funcdef();

		std::string name;

		node_ptr variables;

		bool is_native;

		// Body of the function (non-native)
		node_ptr body;

		// Callback (native)
		typedef std::function<node_ptr(context&, const node_ptr&)> callback;
		callback native_func;

		// Environment
		environment_ptr env;
	};

	struct environment : public std::enable_shared_from_this<environment>
	{
		void register_variable(const std::string& name, node_ptr n);
		node_ptr lookup_variable(const std::string& name);
		void set_variable(const std::string& name, node_ptr n);

		environment_ptr parent;

		typedef std::unordered_map<std::string, node_ptr> var_map;
		var_map bindings;
	};

	std::string type_to_string(slist::node_type type);

	void print_node(const node_ptr& n);

	void debug_print_node(const node_ptr& n, int indent = 0);
	void debug_print_environemnt(const environment_ptr& env);
}

#endif