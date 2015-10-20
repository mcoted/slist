#include "slist_parser.h"
#include "slist_log.h"

#include <cassert>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stack>
#include <iostream>

namespace 
{
	bool get_next_char(std::istream& in, char& ch);
	char peek_next_char(std::istream& in);
	
	bool parse_list(std::istream& in, slist::node_ptr result);
	bool parse_token(std::istream& in, slist::node_ptr& result);
	bool parse_string(std::istream& in, slist::node_ptr& result);
	bool parse_comment(std::istream& in);

	slist::node_type find_type(const std::string& str);
	std::string type_to_string(slist::node_type type);
}

namespace slist
{
	node_ptr parse(const std::string& str)
	{
		std::istringstream in(str);
		in >> std::noskipws;

		node_ptr result(new node);
		result->type = node_type::pair;

		char ch;
		std::string tok;

		while (true)
		{
			if (!get_next_char(in, ch))
			{
				break;
			}

			in.putback(ch);
			if (ch == '(')
			{
				node_ptr list(new node);
				list->type = node_type::pair;

				if (parse_list(in, list))
				{
					result->append(list);
				}
			}
			else if(ch == '"')
			{
				in.putback(ch);
				node_ptr string_node(new node);
				if (parse_string(in, string_node))
				{
					result->append(string_node);
				}
			}
			else if (ch == ';')
			{
				parse_comment(in);
			}
			else
			{
				node_ptr token(new node);
				if (parse_token(in, token))
				{
					result->append(token);
				}
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
	bool get_next_char(std::istream& in, char& ch)
	{
		while (in)
		{
			ch = in.get();
			if (std::isspace(ch) == 0)
			{
				return true;
			}
		}
		return false;
	}

	char peek_next_char(std::istream& in)
	{
		return static_cast<char>(in.peek());
	}

	bool parse_list(std::istream& in, slist::node_ptr result)
	{
		using namespace slist;

		std::string tok;
		char ch = 0;
		bool parsed = false;

		get_next_char(in, ch);
		assert(ch == '(');

		// TODO: This is basically 'parse()' repeated here... call it directly instead?

		while (true)
		{
			if (!get_next_char(in, ch))
			{
				break;
			}

			if (ch == '(')
			{
				in.putback(ch);

				node_ptr list(new node);
				list->type = node_type::pair;

				if (parse_list(in, list))
				{
					result->append(list);
				}
			}
			else if (ch == ')')
			{
				parsed = true;
				break;
			}
			else if(ch == '"')
			{
				in.putback(ch);
				node_ptr string_node(new node);
				if (parse_string(in, string_node))
				{
					result->append(string_node);
				}
			}
			else if (ch == ';')
			{
				parse_comment(in);
			}
			else 
			{
				in.putback(ch);

				node_ptr token(new node);
				if (parse_token(in, token))
				{
					result->append(token);
				}
			}
		}

		if (!parsed)
		{
            log_errorln(std::string("Parsing failed at char: '") + std::string(&ch, 1));
			return false;
		}

		return true;
	}

	bool parse_token(std::istream& in, slist::node_ptr& result)
	{
		using namespace slist;

		char ch = 0;
		if (get_next_char(in, ch))
		{
			in.putback(ch);

			std::string str;

			while (true)
			{
				in.get(ch);

				if (in && ch == '\'')
				{
					result->type = node_type::pair;

					node_ptr sym(new node);
					sym->type = node_type::name;
					sym->value = "'";

					result->append(sym);

					char next_ch = peek_next_char(in);
					if (next_ch == '(')
					{
						node_ptr sublist(new node);
						sublist->type = node_type::pair;

						if (parse_list(in, sublist))
						{
							result->append(sublist);
						}
					}
					else 
					{
						node_ptr subname(new node);
						if (parse_token(in, subname))
						{
							result->append(subname);
						}
					}
                    
                    return true;
				}
				else if (in && ch != '(' && ch != ')' && !std::isspace(ch) && ch != ';')
				{
					str += ch;
				}
				else 
				{
					if (ch == '(' || ch == ')' || ch == ';')
					{
						in.putback(ch);
					}
					break;
				}
			}

            result->type = find_type(str);
            result->value = str;

			return true;
		}

		return false;
	}

	bool parse_string(std::istream& in, slist::node_ptr& result)
	{
		using namespace slist;

		char ch = '\0';
		get_next_char(in, ch);
		if (ch == '"')
		{
			std::string str;

			while (true)
			{
				get_next_char(in, ch);

				if (!in)
				{
					break;
				}

				if (ch == '\\' && peek_next_char(in) == '"')
				{
					get_next_char(in, ch);
					str += ch;
				}
				else if (ch == '"')
				{
					break;
				}

				str += ch;
			}

			result->type = node_type::string;
			result->value = str;

			return true;
		}
		return false;
	}

	bool parse_comment(std::istream& in)
	{
		char ch = '\0';
		while (in && ch != '\n')
		{
			in.get(ch);
		}
		return true;
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
		bool is_first_char = true;

		for (char ch : str)
		{
			if (std::isdigit(ch) || ch == '-')
			{
				has_only_digits = !has_alpha;
			}
			else if (ch == '.')
			{
				++dot_count;
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
		else 
		{
			type = node_type::name;
		}

		return type;
	}
}