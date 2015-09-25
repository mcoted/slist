#include "slist_types.h"
#include <iostream>

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

	void print_node(const node_ptr& n)
	{
		debug_print_node(n);
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