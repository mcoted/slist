#include "slist_types.h"
#include <iostream>

namespace
{
	void print_node_r(const slist::node_ptr& node);
	void debug_print_node_r(const slist::node_ptr& node, int indent);
	std::string type_to_string(slist::node_type type);
}

namespace slist
{
	void print_node(const node_ptr& root)
	{
		print_node_r(root);
		std::cout << '\n';
	}

	void debug_print_node(const node_ptr& root)
	{
		debug_print_node_r(root, 0);
	}
}

namespace
{
	void print_node_r(const slist::node_ptr& node)
	{
		if (node == nullptr)
		{
			std::cout << "<null>";
			return;
		}

		switch (node->type)
		{
			case slist::node_type::empty:
			break;
			case slist::node_type::list:
				if (node->children.size() == 0)
				{
					std::cout << "()";
				}
				else 
				{
					std::cout << "(";
					bool first = true;
					for (const slist::node_ptr& child : node->children)
					{
						if (!first)
						{
							std::cout << " ";							
						}
						print_node_r(child);
						first = false;
					}
					std::cout << ")";
				}
			break;
			default:
				std::cout << node->data;
			break;
		}
	}

	void debug_print_node_r(const slist::node_ptr& node, int indent)
	{
		for (int i = 0; i < indent; ++i) 
		{
			std::cout << "    ";
		}

		if (node == nullptr)
		{
			std::cout << "<null>\n";
			return;
		}

		std::cout << '[' << type_to_string(node->type) << "] ";

		if (node->type == slist::node_type::list)
		{
			std::cout << '\n';
		}

		if (node->data.length() > 0)
		{
			std::cout << node->data << '\n';
		}

		for (const slist::node_ptr& child_node : node->children)
		{
			debug_print_node_r(child_node, indent+1);
		}
	}

	std::string type_to_string(slist::node_type type)
	{
		using namespace slist;

		switch (type)
		{
			case node_type::empty:   return "empty";
			case node_type::list:    return "list";
			case node_type::integer: return "integer";
			case node_type::number:  return "number";
			case node_type::string:  return "string";
			case node_type::symbol:  return "symbol";
		}

		return "<undefined>";
	}
}