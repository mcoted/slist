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

 * Run ```cmake ..``` from the ```build``` folder.
 * run ```make```

 The library will be built in the ```lib``` subfolder, and the interactive
 interpreter will be in ```repl/repl```.

- Building the interactive interpreter on Windows:

 * Run ```cmake ..``` from the ```build``` folder.
 * Open the generated Visual Studio project.

- Embedding SList in your existing project:

 * Simply add the ```.h``` and ```.cpp``` files from the lib
   folder in your project


### Supported Lisp Features

 * Defines and lambdas

 		(define ten 10)
 		(define add-xy (lambda (x y) (+ x y)))
 		(define (mul-xy x y) (* x y))
 		(add-xy 2 3) ; gives 5
 		(mul-xy 2 3) ; gives 6

 * Symbols

 		(define a '(1 2 3))
 		(car a) ; gives 1
 		(cdr a) ; gives (2 3)
 		'a ; gives a

 * Let-constructs

 		(define (add-2 n)
 			(let ((x 2))
 				(+ x n)))
 		(add-2 1) ; gives 3

 * Local environments

 		(define (make-adder n)
 			(lambda (x) (+ n x)))
 		(define add-3 (make-adder 3))
 		(add-3 1) ; gives 4

 * Builtin procedures:

 		eval, apply, cons, list, car, cdr, lambda, define, set!, let, begin,
 		if, length, empty?, print, println, eq?, equal?, not, pair?, boolean?, 
 		integer?, number?, string?, symbol?, +, -, *, /, =, !=, <, >, <=, >=

### Embedding SList in Your Project

##### Evaluating Lisp expressions from your C++ code

The SList functions and classes reside in the ```slist``` namespace.  To evaluate
a Lisp expression, you only need to create a new context, and call ```exec```.

	using namespace slist;
	context ctx;
	exec(ctx, "(println (+ 1 2))");

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
