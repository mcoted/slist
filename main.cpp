#include "slist.h"
#include <cstdlib>

namespace
{
	void parse_arguments(int argc, char **argv);
}

int main(int argc, char **argv)
{
	using namespace slist;

	parse_arguments(argc, argv);

	// parse_node_ptr n = parse("(define a (lambda (x) (+ x 1)))  (a 2)");
	// parse_node_ptr n = parse("(cons 1 2)");
	parse_node_ptr n = parse("(cons 1 (cons 2 3))");
	// parse_node_ptr n = parse("(cons (cons 1 2) (cons 3 4))");

	if (get_log_level() >= log_level::trace)
	{
		log_traceln("");
		debug_print_parse_node(n);
		log_traceln("");
	}

	context ctx;
	for (auto& child : n->children)
	{
		auto val = eval(ctx, child);
		if (val != nullptr)
		{
            auto r = eval(ctx, child);
            if (r != nullptr)
            {
                outputln("", r);
            }
		}
	}
}

namespace
{
	void parse_arguments(int argc, char **argv)
	{
		using namespace std;
		using namespace slist;

		for (int i = 0; i < argc; ++i)
		{
			char *arg = argv[i];
			if (strcmp(arg, "-l") == 0 || strcmp(arg, "--log-level") == 0)
			{
				++i;
				if (i < argc && argv[i] != nullptr)
				{
					arg = argv[i];
					log_level level = (log_level)atoi(arg);
					set_log_level(level);
				}
				else 
				{
					log_error("Invalid argument to '-v'/'--log-level'\n");
				}
			}
		}
	}
}