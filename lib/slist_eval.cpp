#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <istream>

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
	slist::node_ptr eval_name(slist::context& ctx, const slist::node_ptr& root);
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
			case node_type::name:
				result = eval_name(ctx, root);
                break;
			case node_type::integer:
			case node_type::number:
			case node_type::string:
                return root;
			default:
				break;
		}

		log_trace("Result of ", root);
		log_traceln(" -> ", result);

		return result;
	}

	node_ptr eval(context& ctx, const procedure_ptr& f, const node_ptr& args)
	{
		if (f == nullptr)
		{
			return nullptr;
		}

		if (f->is_native)
		{
			// Build a root node
			node_ptr name_node(std::make_shared<node>());
			name_node->type = node_type::name;
			name_node->value = f->name;

			node_ptr root(std::make_shared<node>());
			root->type = node_type::pair;
			root->car = name_node;
			root->cdr = args;

			auto prev_env = ctx.active_env;	
			ctx.active_env = f->env;
			auto result = f->native_func(ctx, root);
			ctx.active_env = prev_env;

			return result;
		}
		else 
		{
			auto prev_env = ctx.active_env;
			ctx.active_env = f->env;
			auto result = eval(ctx, f->body);
			ctx.active_env = prev_env;

			return result;
		}

		return nullptr;
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

	node_ptr exec(context& ctx, std::istream& in)
	{
		std::string s;

		const size_t size = 1024;
		char buffer[1024];
		while (in.read(buffer, size))
		{
			s.append(buffer, size);
		}
		s.append(buffer, in.gcount());

		return exec(ctx, s);
	}

	node_ptr apply(context& ctx, const node_ptr& args, const procedure_ptr& proc)
	{
		// Create a new environement to make sure they are not shared between evals
        environment_ptr env(std::make_shared<environment>());
        env->parent = proc->env;
		proc->env = env;

		node_ptr var = proc->variables;
		node_ptr arg = args;

		if (var != nullptr && var->type == node_type::name)
		{
			// This is a variadic argument, grab all the args
			env->register_variable(var->value, args);
		}
		else 
		{
			while (var != nullptr && arg != nullptr)
			{
				node_ptr var_name = var->car;
				if (var_name == nullptr || var_name->type != node_type::name)
				{
					log_errorln("Invalid variable:\n", var_name);
					return nullptr;
				}

				if (var_name->value == ".")
				{
					// The next argument is a variadic one
					var = var->cdr;
					if (var == nullptr)
					{
						log_errorln("Missing variadic argument after '.'");
						return nullptr;
					}

					var_name = var->car;
					if (var_name == nullptr || var_name->type != node_type::name)
					{
						log_errorln("Invalid variable:\n", var_name);
						return nullptr;
					}

					env->register_variable(var_name->value, arg);
					break;
				}

				env->register_variable(var_name->value, eval(ctx, arg->car));

				arg = arg->cdr;
				var = var->cdr;
			}
		}

		log_traceln("Evaluating Procedure from 'apply':\n", nullptr, proc);
		return eval(ctx, proc, args);
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
        procedure_ptr proc = op_node->proc;

        if (proc == nullptr && op_node->type == node_type::pair)
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
		}

		log_errorln("Operator is not a procedure: ", op_node);

		return nullptr;
	}

	slist::node_ptr eval_name(slist::context& ctx, const slist::node_ptr& root)
	{
		using namespace slist;
		auto var_node = ctx.active_env->lookup_variable(root->value);
		if (var_node == nullptr)
		{
			log_errorln("Could not evaluate variable: ", root);
			return nullptr;
		}
        return var_node;
	}
}