#include "slist_context.h"
#include <iostream>

namespace
{
	slist::node_ptr car(const slist::node_ptr& root);
	slist::node_ptr cdr(const slist::node_ptr& root);
	slist::node_ptr add(const slist::node_ptr& root);
}

namespace slist
{
	context::context()
	{
		global_funcs["car"] = &car;
		global_funcs["cdr"] = &cdr;
		global_funcs["+"] = &add;
	}
}

namespace
{
	slist::node_ptr car(const slist::node_ptr& root)
	{
		using namespace slist;
		if (root->type != node_type::list)
		{
			return nullptr;
		}

		node_ptr result;
		if (root->children.size() == 2)
		{
			auto list = root->children[1];
			if (list->type != node_type::list)
			{
				std::cerr << "'car' expects a list as argument\n";
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				return nullptr;
			}

			return list->children[0];
		}
		else 
		{
			std::cerr << "Invalid number of arguments to 'car'\n";
			return nullptr;
		}

		return result;
	}

	slist::node_ptr cdr(const slist::node_ptr& root)
	{
		using namespace slist;
		if (root->type != node_type::list)
		{
			return nullptr;
		}

		node_ptr result;
		if (root->children.size() == 2)
		{
			auto list = root->children[1];
			if (list->type != node_type::list)
			{
				std::cerr << "'cdr' expects a list as argument\n";
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				return nullptr;
			}

			auto children = 
				std::vector<node_ptr>(list->children.begin()+1,
									  list->children.end());

			node_ptr result(new node);
			result->type = node_type::list;
			result->children = children;
			return result;
		}
		else 
		{
			std::cerr << "Invalid number of arguments to 'cdr'\n";
			return nullptr;
		}

		return result;		
	}

	slist::node_ptr add(const slist::node_ptr& root)
	{
		using namespace slist;

		if (root->children.size() == 0)
		{
			std::cerr << "Not enough arguments to '+'\n";
			return nullptr;
		}

		int intval = 0;
		int floatval = 0.0f;

		node_ptr result_node(new node);
		bool has_floats = false;

		size_t i = 0;
		for (const auto& child : root->children)
		{
			if (i++ == 0)
			{
				continue; // skip '+'
			}

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