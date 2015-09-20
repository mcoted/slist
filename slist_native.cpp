#include "slist_native.h"
#include "slist_context.h"
#include "slist_eval.h"
#include "slist_log.h"
#include <algorithm>

namespace slist
{
	node_ptr ___define(context& ctx, const node_ptr& root)
	{
		if (root->children.size() < 3)
		{
			log_errorln("Invalid arguments for 'define'\n", root);
			return nullptr;
		}

		node_ptr first = root->children[1];
		node_ptr second = root->children[2];

		if (first->type == node_type::list)
		{
			// Lambda shortcut
			node_ptr name = first->children[0];

			funcdef::arg_list arg_list;
			bool variadic = false;

			// TODO: Support actual variadic functions
			if (first->children.size() == 3 &&
				first->children[1]->data == "." &&
				first->children[2]->type == node_type::string)
			{
				arg_list.push_back(first->children[2]->data);
				variadic = true;
			}
			else 
			{
				for (int i = 1; i < first->children.size(); ++i)
				{
					node_ptr arg = first->children[i];
					if (arg->type != node_type::string)
					{
						log_errorln("Invalid function argument:\n", arg);
						return nullptr;
					}
					arg_list.push_back(arg->data);	
				}				
			}

			funcdef_ptr func(new funcdef);
			func->name = name->data;
			func->args = arg_list;
			func->variadic = variadic;
			func->body = second;

			second->proc = func;

			ctx.global_vars[0][func->name] = second;
		}
		else if (first->type == node_type::string)
		{
			ctx.global_vars[0][first->data] = second;
		}

		return nullptr;
	}

	node_ptr ___lambda(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 3)
		{
			log_errorln("Invalid lambda format\n", root);
			return nullptr;
		}

		funcdef::arg_list arg_list;

		// Parse arguments
		node_ptr arg_node = root->children[1];
		if (arg_node->type == node_type::list)
		{
			for (auto& arg : arg_node->children)
			{
				if (arg->type == node_type::string)
				{
					arg_list.push_back(arg->data);
				}
				else 
				{
					log_errorln("Invalid argument:\n", arg);
					return nullptr;
				}
			}
		}
		else if (arg_node->type == node_type::string)
		{
			// TODO: Variadic
		}
		else 
		{
			log_error("Invalid argument type for lambda\n", arg_node);
			return nullptr;
		}

		funcdef_ptr func(new funcdef);
		func->name = root->children[0]->data; // "lambda"
		func->args = arg_list;
		func->variadic = false; // TODO
		func->body = root->children[2];

		log_traceln("Lambda proc:\n", nullptr, func);

		root->proc = func;

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
				log_errorln("'car' expects a list as argument");
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				log_errorln("'car' of empty list");
				return nullptr;
			}

			result = list->children[0];
		}
		else 
		{
			log_errorln("Invalid number of arguments to 'car'");
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
				log_errorln("'cdr' expects a list as argument");
				return nullptr;
			}

			if (list->children.size() == 0)
			{
				log_errorln("'cdr' of empty list");
				return nullptr;
			}

			auto children = 
				std::vector<node_ptr>(list->children.begin()+1,
									  list->children.end());

			result.reset(new node);
			result->type = node_type::list;
			result->children = children;
		}
		else 
		{
			log_errorln("Invalid number of arguments to 'cdr'");
			return nullptr;
		}

        return result;
	}

	node_ptr ___if(context& ctx, const node_ptr& root)
	{
		if (root->children.size() != 4)
		{
			log_errorln("Invalid 'if' statement");
			return nullptr;
		}

		auto pred = eval(ctx, root->children[1]);
		if (pred == nullptr || pred->type != node_type::boolean)
		{
			log_errorln("'empty?' predicate did not evaluate to a boolean value");
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
			log_errorln("Invalid 'empty?' statement");
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
			log_errorln("Invalid 'empty?' statement");
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
			log_errorln("'___add' expects two arguments");
			return nullptr;
		}

		node_ptr first_val = eval(ctx, root->children[1]);
		log_traceln("___add first arg: ", first_val);

		if (first_val == nullptr)
		{
			log_errorln("___add: first arg is null");
			return nullptr;
		}

		node_ptr second_val = eval(ctx, root->children[2]);
		if (second_val == nullptr)
		{
			log_errorln("___add: second arg is null");
			return nullptr;
		}

		if (first_val->type != node_type::integer && 
			first_val->type != node_type::number)
		{
			log_errorln(std::string("___add: invalid first arg type: ") + type_to_string(first_val->type) + "\n", first_val);
			return nullptr;
		}

		if (second_val->type != node_type::integer && 
			second_val->type != node_type::number)
		{
			log_errorln(std::string("___add: invalid second arg type: ") + type_to_string(first_val->type) + "\n", second_val);
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