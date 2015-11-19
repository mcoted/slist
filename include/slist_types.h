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

	struct procedure;
	typedef std::shared_ptr<procedure> procedure_ptr;

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
		node();

		size_t length() const;
		node_ptr get(size_t index);
		void append(const node_ptr& n);
		node_ptr pop();

		bool to_bool() const;
		void set_bool(bool value);

		int to_int() const;
		void set_int(int value);

		float to_float() const;
		void set_float(float value);

		const std::string& to_name() const;
		void set_name(const std::string& name);

		const std::string& to_string() const;
		void set_string(const std::string& str);

		node_type type;
		std::string value;

		node_ptr car;
		node_ptr cdr;

		procedure_ptr proc;
	};

	struct procedure : public std::enable_shared_from_this<procedure>
	{
		procedure();

		std::string name;

		node_ptr variables;

		bool is_native;
		bool is_macro;

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
		bool set_variable(const std::string& name, node_ptr n);

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