#ifndef SLIST_PARSER_H
#define SLIST_PARSER_H

#include "slist_types.h"

namespace slist
{
	tree_ptr parse(const std::string& str);
	tree_ptr parse_stream(std::istream& in);
	tree_ptr parse_file(const std::string& filename);

	void print_tree(const tree_ptr& tree);

	void debug_print_tree(const tree_ptr& tree);
}

#endif