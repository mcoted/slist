#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <iostream>

namespace
{
	slist::parse_node_ptr eval_list(slist::context& ctx, const slist::parse_node_ptr& root);
	slist::parse_node_ptr eval_string(slist::context& ctx, const slist::parse_node_ptr& root);

	bool bind_args(slist::context& ctx, const slist::funcdef::arg_list& args, const slist::parse_node_ptr& root, bool variadic);
	void unbind_args(slist::context& ctx);
}

namespace slist
{
	parse_node_ptr eval(context& ctx, const parse_node_ptr& root)
	{
		log_traceln("Eval: ", root);

        if (root == nullptr)
        {
            return nullptr;
        }
        
        parse_node_ptr result;

		switch (root->type)
		{
			case parse_node_type::list:
				result = eval_list(ctx, root);
                break;
			case parse_node_type::string:
				result =  eval_string(ctx, root);
                break;
			case parse_node_type::integer:
			case parse_node_type::number:
				result = root;
                break;
			default:
				break;
		}

		log_traceln("Result: ", result);

		return result;
	}

	parse_node_ptr exec(context& ctx, const std::string& str)
	{
		parse_node_ptr result;
		parse_node_ptr parse_parse_node = parse(str);
		if (parse_parse_node != nullptr)
		{
			for (auto& child : parse_parse_node->children)
			{
				result = eval(ctx, child);
			}
		}
		return result;
	}
}

namespace
{
	slist::parse_node_ptr eval_list(slist::context& ctx, const slist::parse_node_ptr& root)
	{
		using namespace slist;

		if (root->children.empty())
		{
			return root;
		}

		parse_node_ptr op_parse_node = root->children[0];
		funcdef_ptr proc;

		if (op_parse_node->type == parse_node_type::list)
		{
			op_parse_node = eval(ctx, op_parse_node);
			if (op_parse_node->proc == nullptr)
			{
				log_errorln("Error: first argument is not a procedure", root);
				return nullptr;
			}
			proc = op_parse_node->proc;
		}
		else 
		{
			// Look for globals
			parse_node_ptr val = ctx.lookup_variable(op_parse_node->data);
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
			log_traceln("Evaluating Procedure:", nullptr, proc);

			if (bind_args(ctx, proc->args, root, proc->variadic))
            {
                parse_node_ptr res = eval(ctx, proc->body);
                unbind_args(ctx);
                return res;
            }
		}

		return root;
	}

	slist::parse_node_ptr eval_string(slist::context& ctx, const slist::parse_node_ptr& root)
	{
		slist::parse_node_ptr var_parse_node = ctx.lookup_variable(root->data);
		if (var_parse_node != nullptr)
		{
			return var_parse_node;
		}
		return root;
	}

	bool bind_args(slist::context& ctx, const slist::funcdef::arg_list& args, const slist::parse_node_ptr& root, bool variadic)
	{
		using namespace slist;

		if (variadic)
		{
			if (args.size() != 1)
			{
				log_errorln("Variadic functions should have only one arg");
				return false;
			}

			parse_node::parse_node_vector children;
			if (root->children.size() > 1)
			{
				for (int i = 1; i < root->children.size(); ++i)
				{
                    children.push_back(eval(ctx, root->children[i]));
				}
			}

			parse_node_ptr packed_arg(new parse_node);
			packed_arg->type = parse_node_type::list;
			packed_arg->children = children;

			var_map map;
			map[args[0]] = packed_arg;
			ctx.global_vars.push_back(map);
		}
		else 
		{
			if (args.size() != root->children.size()-1)
			{
				log_errorln("Unable to bind arguments");
				log_error("Args: ");
				for (auto& arg : args)
				{
					log_error(arg + ' ');
				}
				log_errorln("");
				log_error("parse_node: ", root);

				return false;
			}

			log_traceln("Variable bindings:");

			var_map map;
			for (int i = 0; i < args.size(); ++i)
			{
				auto& arg = args[i];
				parse_node_ptr n = eval(ctx, root->children[i+1]);
				map[arg] = n;
				log_trace(arg + ": ", n);
			}

			log_traceln("");
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