#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <iostream>

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
	slist::node_ptr eval_string(slist::context& ctx, const slist::node_ptr& root);

	bool bind_args(slist::context& ctx, slist::environment_ptr env, const slist::funcdef::arg_list& args, const slist::node_ptr& root, bool variadic);
	// void unbind_args(slist::context& ctx);
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

	node_ptr apply(context& ctx, const node_ptr& args, const funcdef_ptr& proc)
	{
		// Clone the environement to make sure they are not shared between evals
        environment_ptr env(new environment(*(proc->env)));
		proc->env = env;

		node_ptr var = proc->variables;
		node_ptr arg = args;
		while (var != nullptr)
		{
			// TODO: Apply 'arg' value to proc's variable in env
			node_ptr var_name = var->car;
			if (var_name->type != node_type::string)
			{
				log_errorln("Invalid variable:\n", var_name);
				return nullptr;
			}

            // TODO: Make sure that we need to evaluate arguments at this point
			env->register_variable(var_name->value, eval(ctx, arg->car));

			arg = arg->cdr;
			var = var->cdr;
		}

		log_traceln("Evaluating Procedure (apply):\n", nullptr, proc);
		auto old_env = ctx.active_env;
		ctx.active_env = proc->env;
		node_ptr res = eval(ctx, proc->body);
		ctx.active_env = old_env;

		return res;
	}
}

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root)
	{
		using namespace slist;

		if (root->length() == 0)
		{
			log_errorln("List is empty", root);
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
            node_ptr val = ctx.global_env->lookup_variable(op_node->value);
            if (val != nullptr && val->proc != nullptr)
            {
                proc = val->proc;
                if (proc != nullptr && proc->is_native)
                {
                    return proc->native_func(ctx, root);
                }
            }
        }

		if (proc != nullptr)
		{
			return apply(ctx, root->cdr, proc);

			// // Clone the environement to make sure they are not shared between evals
			// environment_ptr env(new environment(*(proc->env)));
			// proc->env = env;

			// if (bind_args(ctx, proc->env, proc->args, root->cdr, proc->variadic))
			// {
			// 	log_traceln("Evaluating Procedure:\n", nullptr, proc);
			// 	auto old_env = ctx.active_env;
			// 	ctx.active_env = proc->env;
			// 	node_ptr res = eval(ctx, proc->body);
			// 	ctx.active_env = old_env;
			// 	return res;
			// }
		}

		return root;
	}

	slist::node_ptr eval_string(slist::context& ctx, const slist::node_ptr& root)
	{
		slist::node_ptr var_node = ctx.active_env->lookup_variable(root->value);
		if (var_node != nullptr)
		{
			return var_node;
		}
		return root;
	}

	bool bind_args(slist::context& ctx, slist::environment_ptr env, const slist::funcdef::arg_list& args, const slist::node_ptr& root, bool variadic)
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

			// var_map2 map;
			// map[args[0]] = packed_arg;
			// ctx.global_vars.push_back(map);
            
			env->register_variable(args[0], packed_arg);

            log_trace(std::string("Bind \"") + args[0] + "\": ", env->lookup_variable(args[0]));
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

			for (int i = 0; i < args.size(); ++i)
			{
				auto& arg = args[i];
                node_ptr n = eval(ctx, root->get(i));
				env->register_variable(arg, n);
                log_trace(std::string("Bind \"") + arg + "\": ", n);
			}

			log_traceln("<<<<<");
			log_traceln("");

			// ctx.global_vars.push_back(map);
		}
        
        return true;
	}

	void unbind_args(slist::context& ctx)
	{
		// ctx.global_vars.pop_back();
	}
}