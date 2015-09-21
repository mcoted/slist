#ifndef SLIST_PARSER_H
#define SLIST_PARSER_H

#include "slist_types.h"

namespace slist
{
	parse_node_ptr parse(const std::string& str);
	parse_node_ptr parse_stream(std::istream& in);
	parse_node_ptr parse_file(const std::string& filename);

	void print_parse_node(const parse_node_ptr& root);
	void debug_print_parse_node(const parse_node_ptr& root);
}

#endif