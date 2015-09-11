#include "slist.h"
#include <iostream>

int main()
{
	using namespace slist;

	context ctx;
	node_ptr root = slist::parse("(+ 1 2.0)");
	debug_print_node(eval(ctx, root->children[0]));
	// print_node(root);
	// debug_print_node(root);
}