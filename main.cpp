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

	node_ptr n = parse("((lambda (x) (+ x 1)) 2)");

	if (get_log_level() >= log_level::trace)
	{
		output(n);
		output("\n");
		debug_print_node(n);		
		output("\n");
	}

	context ctx;
	output(eval(ctx, n));
	output("\n");
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