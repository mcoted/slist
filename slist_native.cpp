#include "slist_native.h"
#include "slist_context.h"
#include "slist_eval.h"
#include <algorithm>
#include <iostream>

namespace slist
{
	node_ptr ___define(context& ctx, const node_ptr& root)
	{
		if (root->children.size() < 3)
		{
			std::cerr << "Invalid arguments for 'define'\n";
			return nullptr;
		}

		node_ptr func_name_args = root->children[1];
		if (func_name_args->type != node_type::string &&
			func_name_args->type != node_type::list)
		{
			std::cerr << "Invalid function name:\n";
			print_node(func_name_args);
			return nullptr;
		}

		std::string func_name;
		funcdef::arg_list arg_list;
		node_ptr body;
		bool variadic = false;

		if (func_name_args->type == node_type::list)
		{
			// Variadic func
			if (func_name_args->children.size() != 3                   ||
				func_name_args->children[1]->data != "."               ||
				func_name_args->children[0]->type != node_type::string ||
				func_name_args->children[2]->type != node_type::string)
			{
				std::cerr << "Invalid variadic arguments:\n";
				print_node(func_name_args);
				return nullptr;
			}

			func_name = func_name_args->children[0]->data;
			body = root->children[2];

			arg_list.push_back(func_name_args->children[2]->data);
			variadic = true;
		}
		else if (func_name_args->type == node_type::string)
		{
			// Normal func
			func_name = func_name_args->data;
			auto args = root->children[2];
			body = root->children[3];
			if (args->type != node_type::list)
			{
				std::cerr << "Argument list expected, got:\n";
				print_node(args);
				return nullptr;
			}

			for (auto& arg : args->children)
			{
				if (arg->type != node_type::string)
				{
					std::cerr << "Invalid function argument:\n";
					print_node(arg);
					return nullptr;
				}
				arg_list.push_back(arg->data);
			}
		}

		funcdef_ptr func(new funcdef);
		func->name = func_name;
		func->args = arg_list;
		func->variadic = variadic;
		func->body = body;

		ctx.global_funcs[func->name] = func;

		return root;
	}

	node_ptr ___list(context& ctx, const node_ptr& root)
	{
		auto children = node::node_vector(root->children.begin()+1, root->children.end());

		node_ptr result(new node);
		result->type = node_type::list;
		result->children = children;

		return result;
	}

	node_ptr ___car(context& ctx, const node_ptr& root)
	{
		node_ptr result;
		if (root->children.size() == 2)
		{
			auto list = eval(ctx, root->children[1]);
			if (list->type != node_type::list)
			{
				std::cerr << "'car' expects a list as argument\n";
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				return nullptr;
			}

			return list->children[0];
		}
		else 
		{
			std::cerr << "Invalid number of arguments to 'car'\n";
			return nullptr;
		}

		return result;
	}

	node_ptr ___cdr(context& ctx, const node_ptr& root)
	{
		node_ptr result;
		if (root->children.size() == 2)
		{
			auto list = eval(ctx, root->children[1]);
			if (list->type != node_type::list)
			{
				std::cerr << "'cdr' expects a list as argument\n";
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				return nullptr;
			}

			auto children = 
				std::vector<node_ptr>(list->children.begin()+1,
									  list->children.end());

			node_ptr result(new node);
			result->type = node_type::list;
			result->children = children;

			return result;
		}
		else 
		{
			std::cerr << "Invalid number of arguments to 'cdr'\n";
			return nullptr;
		}

		return result;		
	}

	node_ptr ___if(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 4)
		{
			std::cerr << "Invalid 'if' statement\n";
			return nullptr;
		}

		auto pred = eval(ctx, root->children[1]);
		if (pred == nullptr || pred->type != node_type::boolean)
		{
			std::cerr << "'empty?' predicate did not evaluate to a boolean value\n";
			return nullptr;
		}

		if (pred->to_bool())
		{
			return eval(ctx, root->children[2]);
		}

		return eval(ctx, root->children[3]);
	}

	node_ptr ___length(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 2)
		{
			std::cerr << "Invalid 'empty?' statement\n";
			return nullptr;
		}

		auto arg = eval(ctx, root->children[1]);
		

		node_ptr result(new node);
		result->type = node_type::integer;
		result->data = std::to_string(arg->children.size());

		return result;
	}

	node_ptr ___empty(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 2)
		{
			std::cerr << "Invalid 'empty?' statement\n";
			return nullptr;
		}

		auto arg = eval(ctx, root->children[1]);
		bool is_empty = (arg == nullptr) || arg->children.empty();

		node_ptr result(new node);
		result->type = node_type::boolean;
		result->data = is_empty ? "true" : "false";

		return result;
	}

	node_ptr ___add(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 3)
		{
			std::cerr << "'___add' expects two arguments\n";
			return nullptr;
		}

		node_ptr first_val = eval(ctx, root->children[1]);
		if (first_val == nullptr)
		{
			return nullptr;
		}

		node_ptr second_val = eval(ctx, root->children[2]);
		if (second_val == nullptr)
		{
			return nullptr;
		}

		if (first_val->type != node_type::integer && 
			first_val->type != node_type::number)
		{
			return nullptr;
		}

		if (second_val->type != node_type::integer && 
			second_val->type != node_type::number)
		{
			return nullptr;
		}

		node_ptr result(new node);
		result->type = node_type::integer;

		if (first_val->type == node_type::number ||
			second_val->type == node_type::number)
		{
			result->type = node_type::number;
		}

		if (result->type == node_type::integer)
		{
			result->data = std::to_string(first_val->to_int() + second_val->to_int());
		}
		else 
		{
			result->data = std::to_string(first_val->to_float() + second_val->to_float());
		}

		return result;
	}
}