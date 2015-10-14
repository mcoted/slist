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

        ""

        // "1"

        // "(eval 1)"

        // "(eq? 1 1)"
        // "(eq? 1 2)"

        // "(define value (quote (1 2)))"
        // "(define other-value (quote (1 2)))"
        // "(eq? value value)"
        // "(eq? value other-value)"
        // "(equal? value other-value)"

        // "(+ 1 2 3)"
        // "(- 1 2 3)"
        // "(* 1 2 3)"
        // "(/ 1.0 2 3)"

        // "(= 1 1)"
        // "(= 1 2)"
        // "(< 1 2)"
        // "(<= 2 2)"
        // "(> 1 2)"
        // "(>= 1 2)"

        // "((lambda () 1))"

        // "(define lambda-test (lambda (x y) (+ x y)))"
        // "(lambda-test 1 2)"

        // "(define (make-adder x) (lambda (a) (+ x a)))"
        // "(define add-1 (make-adder 1))"
        // "(define add-2 (make-adder 2))"
        // "(println (add-1 1))"
        // "(println (add-2 1))"
        // "(println (add-1 3))"

        // "(define inner-lambda-test (lambda (x) (lambda (y) (+ x y))))"
        // "((inner-lambda-test 1) 2)"

        // "(define (expanded-let-add-4-test y)"
        // "    ((lambda (x)"
        // "         (+ x y)) 4))"
        // "(expanded-let-add-4-test 6)"

        // "(define (add-4-inside-let y)"
        // "    (let ((x 4))"
        // "        (+ x y)))"
        // "(add-4-inside-let 6)"
        // "(add-4-inside-let 7)"
        // "(add-4-inside-let 6)"

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

        // "(define (variadic-test-4 x . rest) (begin (println x) (apply + rest)))"
        // "(variadic-test-4 9 1 2 3)"

        // "(define variadic-test-5 (lambda (x . rest) (begin (println x) (apply + rest))))"
        // "(variadic-test-5 9 1 2 3)"

        // "(define (make-list value count)"
        // "    (if (> count 0)"
        // "        (cons value (make-list value (- count 1)))"
        // "        (list)))"
        // "(make-list 1 5)"

        // "(define (fact n)"
        // "    (if (<= n 1)"
        // "        1"
        // "        (* n (fact (- n 1)))))"
        // "(fact 6)"

        // "(define (expanded-let-test x) ((lambda (y) (+ x y)) 2))"
        // "(expanded-let-test 1)"

        // "(define let-test-1 (lambda (x) (let ((y 2)) (+ x y))))"
        // "(let-test-1 1)"

        // "(quote a)"
        // "(quote (1 2 3))"
        // "(eval (quote (+ 1 2)))"

        // TODO: 
        //  - Symbols
        //  - Strings
        //  - Comments support
        //  - Tail recursion optimization

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