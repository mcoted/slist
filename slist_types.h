#ifndef SLIST_TYPES_H
#define SLIST_TYPES_H

#include <string>
#include <memory>
#include <vector>

namespace slist
{
	struct node;
	typedef std::shared_ptr<node> node_ptr;

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
		node_type type;
		std::string data;
		std::vector<node_ptr> children;
	};

	void print_node(const slist::node_ptr& node);
	void debug_print_node(const slist::node_ptr& node);
}

#endif