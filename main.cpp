#include "slist.h"

int main()
{
	slist::context context;
	slist::node_ptr root = slist::parse("(+ 1 2.0 'symbol)('hey)");
	slist::print_node(root);
	slist::debug_print_node(root);
}