#include "slist.h"

int main()
{
	using namespace slist;

	node_ptr n = parse("((lambda (x) (+ x 1)) 2)");

	if (get_log_level() >= log_level::trace)
	{
		output(n);
		debug_print_node(n);		
	}

	context ctx;
	output(eval(ctx, n));
}