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
        if (root == nullptr)
        {
            return nullptr;
        }
        
		switch (root->type)
		{
			case node_type::list:
				return eval_list(ctx, root);
			case node_type::string:
				return eval_string(ctx, root);
			case node_type::integer:
			case node_type::number:
				return root;
			default:
				break;
		}

		return nullptr;
	}

	void exec(context& ctx, const std::string& str)
	{
		auto parse_node = parse(str);
		if (parse_node != nullptr)
		{
			for (auto& child : parse_node->children)
			{
				eval(ctx, child);
			}
		}
	}
}

namespace
{
	slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root)
	{
		using namespace slist;

		if (root->children.empty())
		{
			return root;
		}

		node_ptr op_node = eval(ctx, root->children[0]);
		funcdef_ptr proc;

		if (op_node == nullptr)
		{
			log_error("First argument of list evaluated to null\n");
			log_error(root);
			return nullptr;
		}

		if (op_node->type == node_type::list)
		{
			op_node = eval(ctx, op_node);
			if (op_node->proc == nullptr)
			{
				log_error("Error: first argument is not a procedure\n");
				log_error(root);
				return nullptr;
			}
			proc = op_node->proc;
		}
		else 
		{
			// Look for global functions
			auto it_func = ctx.global_funcs.find(op_node->data);
			if (it_func != ctx.global_funcs.end())
			{
				proc = it_func->second;
	        	if (proc->is_native)
	        	{
	        		// Execute native procs right away, no need to bind variables
	        		return proc->native_func(ctx, root);
	        	}
			}
		}

		if (proc != nullptr)
		{
			log_trace("Evaluating Procedure:\n");
			log_trace(proc);
			log_trace("\n");

			if (bind_args(ctx, proc->args, root, proc->variadic))
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
		slist::node_ptr var_node = ctx.lookup_variable(root->data);
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
				log_error("Variadic functions should have only one arg\n");
				return false;
			}

			node::node_vector children;
			if (root->children.size() > 1)
			{
				for (int i = 1; i < root->children.size(); ++i)
				{
                    children.push_back(eval(ctx, root->children[i]));
				}
			}

			node_ptr packed_arg(new node);
			packed_arg->type = node_type::list;
			packed_arg->children = children;

			context::var_map map;
			map[args[0]] = packed_arg;
			ctx.variables.push_back(map);
		}
		else 
		{
			if (args.size() != root->children.size()-1)
			{
				log_error("Unable to bind arguments\n");
				log_error("Args: ");
				for (auto& arg : args)
				{
					log_error(arg + ' ');
				}
				log_error("\n");
				log_error("Node: ");
				log_error(root);

				return false;
			}

			log_trace("Variable bindings:\n");

			context::var_map map;
			for (int i = 0; i < args.size(); ++i)
			{
				auto& arg = args[i];
				node_ptr n = eval(ctx, root->children[i+1]);
				map[arg] = n;
				log_trace(arg + ": ");
				log_trace(n);
			}

			log_trace("\n\n");

			ctx.variables.push_back(map);
		}
        
        return true;
	}

	void unbind_args(slist::context& ctx)
	{
		ctx.variables.pop_back();
	}
}