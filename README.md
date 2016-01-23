    SList - A Lightweight Lisp Compiler
-----------------------------------

SList is a lightweight Lisp compiler written in C++.  It is designed as a
scripting language to be added in existing C++ projects.

 * It supports every basic Lisp concepts.
 * It has no external dependencies.
 * It is easily extensible with your own C++-native functions.
 * It makes it easy to bridge the native types with Lisp's types.


### Installation

SLisp uses [CMake](https://cmake.org) for its build system.

- Building the interactive interpreter on Unix systems:

 * ```./build_unix.sh```
 * The interpreter will be built in ```build/slist```
 * The library will be built in ```build/lib```

 The library will be built in the ```build/lib``` subfolder, and the interactive
 interpreter will be in ```build/slist```.

- Building the interactive interpreter on Windows:

 * ```mkdir build```
 * ```cd build```
 * ```cmake ..```
 * Open the generated Visual Studio project.

- Embedding SList in your existing project:

 * Simply add the ```.h``` files from ```include``` and the ```.cpp``` files 
 from the lib folder in your project


### Supported Lisp Features

 * Defines and lambdas

        (define ten 10)
        (define adder (lambda (x y) (+ x y)))
        (adder 2 3) ; returns 5
        (define (multiplier x y) (* x y)) ; syntactic sugar for lambda definitions
        (multiplier 2 3) ; returns 6

 * Symbols

        (println a)        ; error, unknown variable
        (println 'a)       ; prints a
        (println (1 2 3))  ; error, '1' is not a procedure
        (println '(1 2 3)) ; prints (1 2 3)

 * Let/letrec constructs

        (define (add-2 n)
            (let ((x 2))
                (+ x n)))
        (add-2 1) ; returns 3

 * Local environments

        (define (make-adder n)
            (lambda (x) (+ n x)))
        (define add-3 (make-adder 3))
        (add-3 1) ; returns 4

 * Macros

        (defmacro (foreach var in values body)
           '(letrec ((loop (lambda (x)
                               (if (empty? x)
                                   '()
                                   (let ((,var (car x)))
                                       (begin
                                           (eval ,body)
                                           (loop (cdr x))))))))
               (loop ,values)))

        (foreach a in '(1 2 3) (println a))

        ; prints:
        ; 1
        ; 2
        ; 3

 * Builtin procedures:

        eval, apply, cons, list, car, cdr, lambda, define, defmacro, set!, let, 
        begin, if, length, empty?, print, println, eq?, equal?, not, pair?, boolean?, 
        integer?, number?, string?, symbol?, +, -, *, /, =, !=, <, >, <=, >=

 * Tail call elimination

    Tail calls are eliminated by the SList runtime, which allows deeply recursive 
    loops:

        (define (forever x)
            (begin
                (println x)
                (forever (+ x 1))))

        (forever 0) ; Prints every positive integers


### Embedding SList in Your Project

##### Evaluating Lisp expressions from your C++ code

The SList functions and classes reside in the ```slist``` namespace.  To evaluate
a Lisp expression, you only need to create a new context, and call ```exec```.

    using namespace slist;
    context ctx;
    exec(ctx, "(+ 1 2)");

```exec``` returns the last evaluated node from the expression.  In the previous case, 
it would return a node representing the integer value ```3```.  Nodes are strutures that
can hold any value.  A simplified representation of a node can be this:

    struct node
    {
        node_type type; // may be 'pair', 'boolean', 'integer', 
                        //'number', 'string' or 'name'

        string value; // Every value is encoded in a string at the present time

        node_ptr car; // for pairs
        node_ptr cdr; // for pairs
    };

To get an actual result from an ```exec``` call, you just have to read it from the received
node:

    node_ptr result = exec(ctx, "(+ 1 2)");
    int i = std::atoi(result->value);


##### Calling C++ functions from SList

You can augment the SList environment with your own functions by registering them
in the context using the ```register_native``` method.

    using namespace slist;

    node_ptr my_func(context& ctx, const node_ptr& root)
    {
        // ...
        return nullptr;
    }

    int main(int argc, char *argv[])
    {
        context ctx;
        ctx.register_native("my-func", &my_func); // "my-func" is now part of SList's language
        exec(ctx, "(my-func (+ 1 2))");
        return 0;
    }

The above example will call ```my_func``` with a node representing 
```(my-func (+ 1 2))``` in the ```root``` variable, represented as a list.  

Normally, you would prefer to get the value ```3``` instead of ```(+ 1 2)``` in your function.
To do this, it is your responsibility to call ```eval``` once, and only once, for each argument. When the node represents a list, you can use the ```get``` helper method to get a 
node at the specifed index in the list.

    node_ptr my_func(context& ctx, const node_ptr& root)
    {
        // For example, root may contain (my-func (+ 1 2))
        node_ptr arg = root->get(1); // arg is (+ 1 2)
        arg = eval(ctx, arg); // arg is now 3
        return arg;
    }


### Command-line Usage

When the compilation is completed, you can run the generated ```slist```
executable from the command-line.

##### Basic Usage

To run the interactive interpreter:

    % ./slist

To run the script in ```file.lisp```:

    % ./slist file.lisp

##### Options:

```--exec expr | -e expr```: Evaluate ```expr``` right away, skips interactive prompt. 

Example:

    > ./slist -e "(println 'hi)"
    hi

```--log-level level | -l level```: Sets the logging level. ```level``` can be 1, 2, 3.

 * 1: Errors only
 * 2: Warnings and errors
 * 3: Debug logs

