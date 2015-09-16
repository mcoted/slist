#include "slist.h"
#include <iostream>

int main()
{
	using namespace slist;

	context ctx;
	node_ptr n = parse("(length (list 1 2 3))")->children[0];
	print_node(n);
	print_node(eval(ctx, n));
}