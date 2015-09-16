#include "slist_context.h"
#include "slist_eval.h"
#include "slist_native.h"
#include <algorithm>
#include <iostream>

namespace
{
	const char * builtin___add = 
	"(define (+ . values) (___sum values))"
	"(define (* . values) (___mul values))"
	"(define (- x y) (___sub x y))"
	"(define (/ x y) (___div x y))"
	;
}

namespace slist
{
	context::context()
	{
		// External
		register_native("define", &___define);
		register_native("list",   &___list);
		register_native("car",    &___car);
		register_native("cdr",    &___cdr);
		register_native("if",     &___if);
		register_native("length", &___length);
		register_native("empty?", &___empty);

		// Internal
		register_native("___add", &___add);

		exec(*this, builtin___add);
	}

	void context::register_native(const std::string& name, funcdef::callback func)
	{
		funcdef_ptr f(new funcdef);
		f->is_native = true;
		f->native_func = func;
		global_funcs[name] = f;
	}

	node_ptr context::lookup_variable(const std::string& name)
	{
		if (variables.size() == 0)
		{
			return nullptr;
		}

		for (int i = (int)(variables.size()-1); i >= 0; --i)
		{
			auto stack = variables[i];
			auto it = stack.find(name);
			if (it != stack.end())
			{
				return it->second;
			}
		}

		return nullptr;
	}
}