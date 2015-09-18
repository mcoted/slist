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
			std::cerr << "First argument of list evaluated to null\n";
			print_node(root);
			return nullptr;
		}

		if (op_node->type == node_type::list)
		{
			op_node = eval(ctx, op_node);
			if (op_node->proc == nullptr)
			{
				std::cerr << "Error: first argument is not a procedure\n";
				print_node(root);
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
			std::cout << "Evaluating proc:\n";
			debug_print_funcdef(proc);
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
				std::cerr << "Variadic functions should have only one arg\n";
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
				std::cerr << "Unable to bind arguments\n";
				std::cerr << "Args: ";
				for (auto& arg : args)
				{
					std::cerr << arg << ' ';
				}
				std::cerr << '\n';
				std::cerr << "Node: ";
				print_node(root);
				return false;
			}

			std::cout << "Variable bindings:\n";

			context::var_map map;
			for (int i = 0; i < args.size(); ++i)
			{
				auto& arg = args[i];
				node_ptr n = eval(ctx, root->children[i+1]);
				map[arg] = n;
				std::cout << arg << ": ";
				print_node(n);
			}
			ctx.variables.push_back(map);
		}
        
        return true;
	}

	void unbind_args(slist::context& ctx)
	{
		ctx.variables.pop_back();
	}
}