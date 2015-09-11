#ifndef SLIST_PARSER_H
#define SLIST_PARSER_H

#include "slist_types.h"

namespace slist
{
	node_ptr parse(const std::string& str);
	node_ptr parse_stream(std::istream& in);
	node_ptr parse_file(const std::string& filename);

	void print_node(const node_ptr& root);
	void debug_print_node(const node_ptr& root);
}

#endif