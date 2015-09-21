#include "slist_context.h"
#include "slist_eval.h"
#include "slist_native.h"
#include "slist_log.h"
#include <algorithm>

namespace
{
	const char * builtin___add = 
	"(define (+ . values) (___sum values))"
	"(define (* . values) (___mul values))"
	""
	"(define (___sum values)"
	"    (if (empty? values)"
	"        0"
	"        (___add (car values) (___sum (cdr values)))))"
	""
	"(define (___mul values)"
	"    (if (empty? values)"
	"        1"
	"        (___mul (car values) (___sum (cdr values)))))"
	;
}

namespace slist
{
	context::context()
	{
		// Prepare global variables map
		var_map map;
		global_vars.push_back(map);

		// External
		register_native("define", &___define);
		register_native("lambda", &___lambda);
		register_native("cons",   &___cons);
		register_native("list",   &___list);
		register_native("car",    &___car);
		register_native("cdr",    &___cdr);
		register_native("if",     &___if);
		register_native("length", &___length);
		register_native("empty?", &___empty);

		// Internal
		register_native("___add", &___add);

		// Execute the builtins script to register the builtin procedures
		exec(*this, builtin___add);
	}

	void context::register_native(const std::string& name, funcdef::callback func)
	{
		funcdef_ptr f(new funcdef);
		f->is_native = true;
		f->native_func = func;

		node_ptr n(new node);
		n->proc = f;

		global_vars.back()[name] = n;
	}

	node_ptr context::lookup_variable(const std::string& name)
	{
		log_traceln("Lookup variable: " + name);

		if (global_vars.size() == 0)
		{
			log_traceln("Not found");
			return nullptr;
		}

		for (int i = (int)(global_vars.size()-1); i >= 0; --i)
		{
			auto stack = global_vars[i];
			auto it = stack.find(name);
			if (it != stack.end())
			{
				log_traceln(name + " = ", it->second);
				return it->second;
			}
		}

		log_traceln("Not found");

		return nullptr;
	}
}