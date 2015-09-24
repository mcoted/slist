#include "slist_types.h"
#include <iostream>

namespace
{
	void print_parse_node_r(const slist::parse_node_ptr& parse_node);
	void debug_print_parse_node_r(const slist::parse_node_ptr& parse_node, int indent);
}

namespace slist
{
	size_t node::length() const
	{
		size_t len = 0;
		const node *p = this;
		while (p != nullptr)
		{
			p = p->cdr.get();
			++len;
		}
		return len;
	}

	node_ptr node::get(size_t index)
	{
		node_ptr p = shared_from_this();
		while (index > 0 && p != nullptr)
		{
			p = p->cdr;
			--index;
		}
		return p != nullptr ? p->car : p;
	}

	void node::append(const node_ptr& n)
	{
		node *p = this;

        if (p->car == nullptr)
        {
            p->car = n;
            return;
        }
        
        while (p->cdr != nullptr)
		{
			if (p->car == nullptr)
			{
				p->car = n;
				return;
			}
			p = p->cdr.get();
		}

		node_ptr next(new node);
        next->type = node_type::pair;
		next->car = n;
		p->cdr = next;
	}

	bool node::to_bool() const 
	{
		if (type != node_type::boolean)
		{
			std::cerr << "Cannot convert to boolean, invalid type: " << (int)type << '\n';
			return false;
		}
		std::string v = value;
		std::transform(v.begin(), v.end(), v.begin(), ::tolower);
		return v == "true";
	}

	int node::to_int() const 
	{
		if (type != node_type::integer) 
		{
			std::cerr << "Cannot convert node to int, invalid type: " << (int)type << '\n';
			return 0;
		}
		return std::stoi(value);
	}

	float node::to_float() const
	{
		if (type != node_type::integer && 
			type != node_type::number)
		{
			std::cerr << "Cannot convert parse_node to float, invalid type: " << (int)type << '\n';
			return 0;			
		}
		return std::stof(value);
	}

	bool parse_node::to_bool() const 
	{
		if (type != node_type::boolean)
		{
			std::cerr << "Cannot convert to boolean, invalid type: " << (int)type << '\n';
			return false;
		}
		std::string v = data;
		std::transform(v.begin(), v.end(), v.begin(), ::tolower);
		return v == "true";
	}

	int parse_node::to_int() const 
	{
		if (type != node_type::integer) 
		{
			std::cerr << "Cannot convert parse_node to int, invalid type: " << (int)type << '\n';
			return 0;
		}
		return std::stoi(data);
	}

	float parse_node::to_float() const
	{
		if (type != node_type::integer && 
			type != node_type::number)
		{
			std::cerr << "Cannot convert parse_node to float, invalid type: " << (int)type << '\n';
			return 0;			
		}
		return std::stof(data);
	}

	void print_node(const node_ptr& n)
	{
		debug_print_node(n);
	}

	void print_parse_node(const parse_node_ptr& root)
	{
		print_parse_node_r(root);
		std::cout << '\n';
	}

	void debug_print_parse_node(const parse_node_ptr& root)
	{
		debug_print_parse_node_r(root, 0);
	}

	void debug_print_funcdef(const funcdef_ptr& func)
	{
		std::cout << "Func: " << func->name << '\n';

		std::cout << "Args: ";
		for (auto& arg : func->args)
		{
			std::cout << arg << ' ';
		}
		if (func->variadic)
		{
			std::cout << " (variadic)";
		}
		std::cout << '\n';

		std::cout << "Body: ";
		print_node(func->body);
	}

	void debug_print_node(const node_ptr& n, int indent)
	{
		for (int i = 0; i < indent; ++i)
		{
			std::cout << "    ";
		}

		if (n == nullptr)
		{
			std::cout << "nil\n";
            return;
		}

		std::cout << "[" << type_to_string(n->type) << "]";

		if (n->value.length() > 0)
		{
			std::cout << " \"" << n->value << "\"";
		}

		std::cout << '\n';

		if (n->type == node_type::pair)
		{
			debug_print_node(n->car, indent+1);
			debug_print_node(n->cdr, indent+1);
		}
	}

	std::string type_to_string(slist::node_type type)
	{
		using namespace slist;

		switch (type)
		{
			case node_type::empty:   return "empty";
			case node_type::list:    return "list";
			case node_type::pair:    return "pair";
			case node_type::boolean: return "boolean";
			case node_type::integer: return "integer";
			case node_type::number:  return "number";
			case node_type::string:  return "string";
			case node_type::symbol:  return "symbol";
		}

		return "<undefined>";
	}
}

namespace
{
	void print_parse_node_r(const slist::parse_node_ptr& parse_node)
	{
		if (parse_node == nullptr)
		{
			std::cout << "<null>";
			return;
		}

		switch (parse_node->type)
		{
			case slist::node_type::empty:
			break;
			case slist::node_type::list:
				if (parse_node->children.size() == 0)
				{
					std::cout << "()";
				}
				else 
				{
					std::cout << "(";
					bool first = true;
					for (const slist::parse_node_ptr& child : parse_node->children)
					{
						if (!first)
						{
							std::cout << " ";							
						}
						print_parse_node_r(child);
						first = false;
					}
					std::cout << ")";
				}
			break;
			default:
				std::cout << parse_node->data;
			break;
		}
	}

	void debug_print_parse_node_r(const slist::parse_node_ptr& parse_node, int indent)
	{
		for (int i = 0; i < indent; ++i) 
		{
			std::cout << "    ";
		}

		if (parse_node == nullptr)
		{
			std::cout << "<null>\n";
			return;
		}

		std::cout << '[' << type_to_string(parse_node->type) << "] ";

		if (parse_node->type == slist::node_type::list)
		{
			std::cout << '\n';
		}

		if (parse_node->data.length() > 0)
		{
			std::cout << parse_node->data << '\n';
		}

		for (const slist::parse_node_ptr& child_parse_node : parse_node->children)
		{
			debug_print_parse_node_r(child_parse_node, indent+1);
		}
	}
}