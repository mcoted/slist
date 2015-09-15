#ifndef SLIST_TYPES_H
#define SLIST_TYPES_H

#include <string>
#include <memory>
#include <vector>

namespace slist
{
	struct node;
	typedef std::shared_ptr<node> node_ptr;

	struct funcdef;
	typedef std::shared_ptr<funcdef> funcdef_ptr;

	enum class node_type
	{
		empty,
		list,
		integer,
		number,
		string,
		symbol
	};

	struct node
	{
		node() : type(node_type::empty) { }

		int to_int() const;
		float to_float() const;

		node_type type;
		std::string data;

		typedef std::vector<node_ptr> node_vector;
		node_vector children;
	};

	struct funcdef
	{
		funcdef() : variadic(false) {}
		
		std::string name;

		typedef std::vector<std::string> arg_list;
		arg_list args;
		bool variadic;

		node_ptr body;
	};

	void print_node(const node_ptr& node);
	void debug_print_node(const node_ptr& node);
	void debug_print_funcdef(const funcdef_ptr& func);
}

#endif