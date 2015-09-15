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
}

namespace slist
{
	node_ptr parse(const std::string& str)
	{
		std::istringstream in(str);

		node_ptr result(new node);
		result->type = node_type::list;

		char ch;
		std::string tok;

		while (true)
		{
			in >> ch;

			if (in && ch == '(')
			{
				in.putback(ch);
				result->children.push_back(parse_list(in));
			}
			else 
			{
				// TODO: throw?
				break;
			}
		}

		return result;
	}

	node_ptr parse_stream(std::istream& in)
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

	node_ptr parse_file(const std::string& filename)
	{
		std::ifstream in(filename);
		return parse_stream(in);
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
            std::cerr << "Parsing failed at char: '" << ch << "'\n";
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

		if (str == "true" || str == "false")
		{
			return node_type::boolean;
		}

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
}