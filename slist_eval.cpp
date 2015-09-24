#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <iostream>

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
	slist::node_ptr eval_string(slist::context& ctx, const slist::node_ptr& root);

	bool bind_args(slist::context& ctx, const slist::funcdef::arg_list& args, const slist::node_ptr& root, bool variadic);
	void unbind_args(slist::context& ctx);
}

namespace slist
{
	node_ptr eval(context& ctx, const node_ptr& root)
	{
		log_traceln("Eval: ", root);

        if (root == nullptr)
        {
            return nullptr;
        }
        
        node_ptr result;

		switch (root->type)
		{
			case node_type::pair:
				result = eval_list(ctx, root);
                break;
			case node_type::string:
				result =  eval_string(ctx, root);
                break;
			case node_type::integer:
			case node_type::number:
				result = root;
                break;
			default:
				break;
		}

		log_traceln("Result: ", result);

		return result;
	}

	node_ptr exec(context& ctx, const std::string& str)
	{
		node_ptr result;
		node_ptr parse_node = parse(str);
		if (parse_node != nullptr)
		{
			while (parse_node != nullptr)
			{
				result = eval(ctx, parse_node->car);
				parse_node = parse_node->cdr;
			}
		}
		return result;
	}
}

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root)
	{
		using namespace slist;

		if (root->length() == 0)
		{
			log_errorln("List is not a procedure", root);
			return nullptr;
		}

		node_ptr op_node = root->get(0);
		funcdef_ptr proc;

		if (op_node->type == node_type::pair)
		{
			node_ptr eval_node = eval(ctx, op_node);
			if (eval_node == nullptr || eval_node->proc == nullptr)
			{
				log_errorln("Error: first argument is not a procedure", root);
				return nullptr;
			}
			proc = eval_node->proc;
		}
		else 
		{
			// Look for globals
			node_ptr val = ctx.lookup_variable(op_node->value);
			if (val != nullptr && val->proc != nullptr)
			{
				proc = val->proc;
				if (proc != nullptr && proc->is_native)
				{
					// Execute native procs right away, no need to bind variables
					return proc->native_func(ctx, root);
				}
			}
		}

		if (proc != nullptr)
		{
			log_traceln("Evaluating Procedure:\n", nullptr, proc);

			if (bind_args(ctx, proc->args, root->cdr, proc->variadic))
			{
				node_ptr res = eval(ctx, proc->body);
				unbind_args(ctx);
				return res;
			}
		}

		return root;
	}

	slist::node_ptr eval_string(slist::context& ctx, const slist::node_ptr& root)
	{
		slist::node_ptr var_node = ctx.lookup_variable(root->value);
		if (var_node != nullptr)
		{
			return var_node;
		}
		return root;
	}

	bool bind_args(slist::context& ctx, const slist::funcdef::arg_list& args, const slist::node_ptr& root, bool variadic)
	{
		using namespace slist;

		if (variadic)
		{
			if (args.size() != 1)
			{
				log_errorln("Variadic functions should have only one arg");
				return false;
			}

			node_ptr packed_arg(new node);
			packed_arg->type = node_type::pair;

			if (root->length() > 0)
			{
				node_ptr p = root;
				while (p != nullptr)
				{
					packed_arg->append(eval(ctx, p->car));
					p = p->cdr;
				}
			}

            log_traceln("\n>>>> BIND ARGS (VARIADIC):");

			var_map2 map;
			map[args[0]] = packed_arg;
			ctx.global_vars.push_back(map);
            
            log_trace(std::string("Bind \"") + args[0] + "\": ", map[args[0]]);
            log_traceln("<<<<<");
            log_traceln("");

		}
		else 
		{
			if (args.size() != root->length())
			{
				log_errorln("Unable to bind arguments");
				log_error("Args: ");
				for (auto& arg : args)
				{
					log_error(arg + ' ');
				}
				log_errorln("");
				log_error("node: ", root);

				return false;
			}

			log_traceln("\n>>>> BIND ARGS:");

			var_map2 map;
			for (int i = 0; i < args.size(); ++i)
			{
				auto& arg = args[i];
                node_ptr n = eval(ctx, root->get(i));
				map[arg] = n;
                log_trace(std::string("Bind \"") + arg + "\": ", n);
			}

			log_traceln("<<<<<");
			log_traceln("");

			ctx.global_vars.push_back(map);
		}
        
        return true;
	}

	void unbind_args(slist::context& ctx)
	{
		ctx.global_vars.pop_back();
	}
}