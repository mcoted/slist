#include "slist.h"
#include <cstdlib>

#include <mach/mach.h>
#include <mach/mach_vm.h>

namespace
{
	void parse_arguments(int argc, char **argv);
}

int main(int argc, char **argv)
{
	using namespace slist;
    
	parse_arguments(argc, argv);

	// parse_node_ptr n = parse("(define a (lambda (x) (+ x 1)))  (a 2)");
	// node_ptr n = parse("(define a (cons (cons 1 2) (cons 3 4))) a");
	node_ptr n = parse(

		"(define (make-adder x) (lambda (a) (___add x a)))"
		"(define add-1 (make-adder 1))"
		"(define add-2 (make-adder 2))"
		"(println (add-1 1))"
		"(println (add-2 1))"
		"(println (add-1 3))"

		// "(define (add x y) (___add x y))"
		// "(apply add (list 1 2))"

		// "(define (f x y) (begin (println x) (println y)))"
		// "(f 1 2)"

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