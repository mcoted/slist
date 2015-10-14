#include "slist_context.h"
#include "slist_eval.h"
#include "slist_native.h"
#include "slist_log.h"
#include <algorithm>

namespace
{
	// const char * builtin___add = 
	// "(define (+ . values) (___sum values))"
	// "(define (___sum values)"
	// "    (if (empty? values)"
	// "        0"
	// "        (___add (car values) (___sum (cdr values)))))"
	// ;
}

namespace slist
{
	context::context()
	{
		// Prepare global environment
		global_env.reset(new environment);
		active_env = global_env;

		register_native("define",  &___define);
		register_native("lambda",  &___lambda);
		register_native("eval",    &___eval);
		register_native("apply",   &___apply);
		register_native("cons",    &___cons);
		register_native("list",    &___list);
		register_native("car",     &___car);
		register_native("cdr",     &___cdr);
		register_native("quote",   &___quote);
		register_native("let",     &___let);
		register_native("begin",   &___begin);
		register_native("if",      &___if);
		register_native("length",  &___length);
		register_native("empty?",  &___empty);
		register_native("print",   &___print);
		register_native("println", &___println);
		register_native("eq?",     &___eq);
		register_native("equal?",  &___equal);
		register_native("not",     &___not);

		register_native("+", &___add);
		register_native("-", &___sub);
		register_native("*", &___mul);
		register_native("/", &___div);

		register_native("=",  &___e);
		register_native("!=", &___ne);
		register_native("<",  &___lt);
		register_native(">",  &___gt);
		register_native("<=", &___le);
		register_native(">=", &___ge);

		register_native("assert", &___assert);

		// Execute the builtins script to register the builtin procedures
		//exec(*this, builtin___add);
	}

	void context::register_native(const std::string& name, funcdef::callback func)
	{
		funcdef_ptr f(new funcdef);
		f->env->parent = active_env;
		f->is_native = true;
		f->native_func = func;

		node_ptr n(new node);
		n->proc = f;

		global_env->register_variable(name, n);
	}
}