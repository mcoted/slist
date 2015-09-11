#include "slist_parser.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

namespace 
{
	slist::node_ptr parse_list(std::istream& in);
	slist::node_ptr parse_token(std::istream& in);
	slist::node_type find_type(const std::string& str);
	std::string type_to_string(slist::node_type type);

	void print_node(const slist::node_ptr& node);
	void debug_print_node(const slist::node_ptr& node, int indent = 0);
}

namespace slist
{
	tree_ptr parse(const std::string& str)
	{
		std::istringstream in(str);

		tree_ptr result(new tree);

		char ch;
		std::string tok;

		while (true)
		{
			in >> ch;

			if (in && ch == '(')
			{
				in.putback(ch);
				result->roots.push_back(parse_list(in));
			}
			else 
			{
				// TODO: throw?
				break;
			}
		}

		return result;
	}

	tree_ptr parse_stream(std::istream& in)
	{
		const size_t bufsize = 1024;
		char buf[bufsize];

		std::string str;

		while (!in.eof())
		{
			in.read(buf, bufsize);
			str += std::string(buf, in.gcount());
		}

		return parse(str);
	}

	tree_ptr parse_file(const std::string& filename)
	{
		std::ifstream in(filename);
		return parse_stream(in);
	}

	void print_tree(const tree_ptr& tree)
	{
		if (tree == nullptr)
		{
			return;
		}

		for (node_ptr& node : tree->roots)
		{
			print_node(node);
		}
	}

	void debug_print_tree(const tree_ptr& tree)
	{
		if (tree == nullptr)
		{
			return;
		}

		for (const node_ptr& node : tree->roots)
		{
			debug_print_node(node);
		}
	}
}

namespace 
{
	slist::node_ptr parse_list(std::istream& in)
	{
		using namespace slist;
		node_ptr result(new node);
		result->type = node_type::list;

		std::string tok;
		char ch = 0;
		bool parsed = false;

		in >> ch;
		assert(ch == '(');

		while (true)
		{
			in >> ch;

			if (!in)
			{
				break;
			}

			if (ch == '(')
			{
				in.putback(ch);
				node_ptr child = parse_list(in);
				result->children.push_back(child);
			}
			else if (ch == ')')
			{
				parsed = true;
				break;
			}
			else 
			{
				in.putback(ch);
				slist::node_ptr child = parse_token(in);
				result->children.push_back(child);
			}
		}

		if (!parsed)
		{
			return nullptr;
		}

		return result;
	}

	slist::node_ptr parse_token(std::istream& in)
	{
		char ch = 0;
		in >> ch;

		if (in)
		{
			in.putback(ch);

			std::string str;

			while (true)
			{
				in.get(ch);
			
				if (in && ch != '(' && ch != ')' && !std::isspace(ch))
				{
					str += ch;
				}
				else 
				{
					if (ch == '(' || ch == ')')
					{
						in.putback(ch);
					}
					break;
				}
			}

			slist::node_ptr node(new slist::node);
			node->data = str;
			node->type = find_type(str);

			return node;
		}

		return nullptr;
	}

	slist::node_type find_type(const std::string& str)
	{
		using namespace slist;
		node_type type = node_type::empty;

		int dot_count = 0;
		bool has_only_digits = false;
		bool has_alpha = false;
		bool is_symbol = false;
		bool is_first_char = true;

		for (char ch : str)
		{
			if (std::isdigit(ch))
			{
				has_only_digits = !has_alpha;
			}
			else if (ch == '.')
			{
				++dot_count;
			}
			else if (ch =='\'' && str.length() > 0)
			{
				is_symbol = is_first_char;
				has_alpha = true;
			}
			else
			{
				has_alpha = true;
				has_only_digits = false;
			}

			is_first_char = false;
		}

		if (has_only_digits && dot_count == 1)
		{
			type = node_type::number;
		}
		else if (has_only_digits && dot_count == 0)
		{
			type = node_type::integer;
		}
		else if (is_symbol)
		{
			type = node_type::symbol;
		}
		else 
		{
			type = node_type::string;
		}

		return type;
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

	void print_node(const slist::node_ptr& node)
	{
		switch (node->type)
		{
			case slist::node_type::empty:
			break;
			case slist::node_type::list:
				if (node->children.size() == 0)
				{
					std::cout << "()\n";
				}
				else 
				{
					std::cout << "( ";
					for (const slist::node_ptr& child : node->children)
					{
						print_node(child);
						std::cout << " ";
					}
					std::cout << ")\n";
				}
			break;
			default:
				std::cout << node->data;
			break;
		}
	}

	void debug_print_node(const slist::node_ptr& node, int indent)
	{
		if (node == nullptr)
		{
			return;
		}

		for (int i = 0; i < indent; ++i) 
		{
			std::cout << "    ";
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
			debug_print_node(child_node, indent+1);
		}
	}
}