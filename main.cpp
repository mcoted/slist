#include "slist.h"
#include <iostream>

int main()
{
	using namespace slist;

	context ctx;
	node_ptr root = slist::parse("(cdr (+ 1 (* 3 4)))");
	print_node(eval(ctx, root->children[0]));
	// print_node(root);
	// debug_print_node(root);
}