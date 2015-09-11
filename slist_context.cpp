#include "slist_context.h"
#include <iostream>

namespace
{
	slist::node_ptr add(const slist::node_ptr& node);
}

namespace slist
{
	context::context()
	{
		global_funcs["+"] = &add;
	}
}

namespace
{
	slist::node_ptr add(const slist::node_ptr& root_node)
	{
		using namespace slist;

		if (root_node->children.size() == 0)
		{
			std::cerr << "Not enough arguments to '+'\n";
			return nullptr;
		}

		int intval = 0;
		int floatval = 0.0f;

		node_ptr result_node(new node);
		bool has_floats = false;

		for (const auto& child : root_node->children)
		{
			if (child->type == node_type::integer)
			{
				intval += std::stoi(child->data);
			}
			else if (child->type == node_type::number)
			{
				has_floats = true;
				floatval += std::stof(child->data);
			}
			else
			{
				std::cerr << "Invalid argument to '+'\n";
				return nullptr;
			}
		}

		if (has_floats)
		{
			result_node->type = node_type::number;
			result_node->data = std::to_string(floatval+intval);
		}
		else 
		{
			result_node->type = node_type::integer;
			result_node->data = std::to_string(intval);			
		}

		return result_node;
	}
}