#include "slist.h"
#include <iostream>

int main()
{
	using namespace slist;

	context ctx;
	node_ptr n = eval(ctx, parse("(+ 1 2)")->children[0]);
	// print_node(eval(ctx, root->children[0]));
	print_node(n);
	//debug_print_node(root);
}