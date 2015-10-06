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

	node_ptr n = parse(

		// "(+ 1 2 3)"
		// "(- 1 2 3)"
		// "(* 1 2 3)"
		// "(/ 1.0 2 3)"

		// "(define (make-adder x) (lambda (a) (___add x a)))"
		// "(define add-1 (make-adder 1))"
		// "(define add-2 (make-adder 2))"
		// "(println (add-1 1))"
		// "(println (add-2 1))"
		// "(println (add-1 3))"

		// "(define (add x y) (+ x y))"
		// "(apply add (list 1 2))"

		// "(define (f x y) (begin (println x) (println y)))"
		// "(f 1 2)"

		// "(define variadic-test-1 (lambda values (apply + values)))"
		// "(variadic-test-1 1 2 3)"

		// "(define my-add (lambda values (apply + values)))"
		// "(define variadic-test-2 (lambda values (apply my-add values)))"
		// "(variadic-test-2 1 2 3)"

		// "(define (variadic-test-3 . values) (apply + values))"
		// "(variadic-test-3 1 2 3)"

		"(define (variadic-test-4 x . rest) (begin (println x) (apply + rest)))"
		"(variadic-test-4 9 1 2 3)"

		);

	if (get_log_level() >= log_level::trace)
	{
		log_traceln("");
		debug_print_node(n);
		log_traceln("");
	}

	context ctx;
	while (n != nullptr)
	{
		auto r = eval(ctx, n->car);
		if (r != nullptr)
		{
			outputln("", r);
		}
		n = n->cdr;
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