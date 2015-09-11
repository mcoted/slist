#include "slist.h"

int main()
{
	slist::context context;
	slist::tree_ptr tree = slist::parse("(+ 1 2.0 'symbol)('hey)");
	print_tree(tree);
	debug_print_tree(tree);
}