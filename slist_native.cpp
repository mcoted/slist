#include "slist_native.h"
#include "slist_context.h"
#include "slist_eval.h"
#include "slist_log.h"
#include <algorithm>

namespace slist
{
	node_ptr ___define(context& ctx, const node_ptr& root)
	{
		if (root->length() < 3)
		{
			log_errorln("Invalid arguments for 'define'\n", root);
			return nullptr;
		}

		node_ptr first = root->get(1);
		node_ptr second = root->get(2);

		if (first->type == node_type::pair)
		{
			// Lambda shortcut
			node_ptr name = first->get(0);

			funcdef::arg_list arg_list;
			bool variadic = false;

			// TODO: Support actual variadic functions
			if (first->length() == 3 &&
				first->get(1)->value == "." &&
				first->get(2)->type == node_type::string)
			{
				arg_list.push_back(first->get(2)->value);
				variadic = true;
			}
			else 
			{
				for (int i = 1; i < first->length(); ++i)
				{
					node_ptr arg = first->get(i);
					if (arg == nullptr || arg->type != node_type::string)
					{
						log_errorln("Invalid function argument:\n", arg);
						return nullptr;
					}
					arg_list.push_back(arg->value);	
				}				
			}

			funcdef_ptr func(new funcdef);
			func->name = name->value;
			func->args = arg_list;
			func->variadic = variadic;
			func->body = second;

			second->proc = func;

            ctx.global_vars[0][func->name] = second;
		}
		else if (first->type == node_type::string)
		{
            ctx.global_vars[0][first->value] = eval(ctx, second);
		}

		return nullptr;
	}

	node_ptr ___lambda(context& ctx, const node_ptr& root)
	{
		if (root->proc != nullptr)
		{
			log_warningln("Evaluating a lambda that already has a procedure. Was it evaluated twice?");
			return root; // Already evaluated
		}

		if (root->length() != 3)
		{
			log_errorln("Invalid lambda format\n", root);
			return nullptr;
		}

		funcdef::arg_list arg_list;

		// Parse arguments
		node_ptr arg_node = root->get(1);
		if (arg_node->type == node_type::pair)
		{
			while (arg_node != nullptr)
			{
				node_ptr arg = arg_node->car;
				if (arg != nullptr && arg->type == node_type::string)
				{
					arg_list.push_back(arg_node->car->value);				
				}
				else 
				{
					log_errorln("Invalid argument:\n", arg);
					return nullptr;	
				}
				arg_node = arg_node->cdr;
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
		func->name = root->get(0)->value; // "lambda"
		func->args = arg_list;
		func->variadic = false; // TODO
		func->body = root->get(2);

		log_traceln("Lambda proc:\n", nullptr, func);

		root->proc = func;

		return root;
	}

	node_ptr ___cons(context& ctx, const node_ptr& root)
	{
		if (root->length() != 3)
		{
			log_errorln("Invalid arguments to 'cons':\n", root);
			return nullptr;
		}

		node_ptr result(new node);
		result->type = node_type::pair;
		result->car = eval(ctx, root->get(1));
		result->cdr = eval(ctx, root->get(2));

		return result;
	}

	node_ptr ___list(context& ctx, const node_ptr& root)
	{
		if (root->length() == 1)
		{
			node_ptr result(new node);
			result->type = node_type::empty;
			return result;
		}
		else 
		{
			return root->cdr;
		}
	}

	node_ptr ___car(context& ctx, const node_ptr& root)
	{
		if (root->length() < 2 || root->get(1) == nullptr)
		{
			log_errorln("Invalid pair for 'car': ", root);
			return nullptr;
		}

        node_ptr n = eval(ctx, root->get(1));
        if (n != nullptr && n->type == node_type::pair)
        {
            return n->car;
        }

        return nullptr;
	}

	node_ptr ___cdr(context& ctx, const node_ptr& root)
	{
		if (root->length() < 2 || root->get(1) == nullptr)
		{
			log_errorln("Invalid pair for 'car': ", root);
			return nullptr;
		}

        node_ptr n = eval(ctx, root->get(1));
        if (n != nullptr && n->type == node_type::pair && n->cdr != nullptr)
        {
            return n->cdr;
        }
        
        node_ptr empty(new node);
        empty->type = node_type::pair;

        return empty;
	}

	node_ptr ___begin(context& ctx, const node_ptr& root)
	{
		node_ptr n = root->cdr;
		node_ptr result;
		while (n != nullptr)
		{
			result = eval(ctx, n->car);
			n = n->cdr;
		}

		if (result == nullptr)
		{
			result.reset(new node);
			result->type = node_type::pair;
		}

		return result;
	}

	node_ptr ___if(context& ctx, const node_ptr& root)
	{
		if (root->length() != 4)
		{
			log_errorln("Invalid 'if' statement");
			return nullptr;
		}

		auto pred = eval(ctx, root->get(1));
		if (pred == nullptr || pred->type != node_type::boolean)
		{
			log_errorln("'empty?' predicate did not evaluate to a boolean value");
			return nullptr;
		}

		if (pred->to_bool())
		{
			return eval(ctx, root->get(2));
		}

		return eval(ctx, root->get(3));
	}

	node_ptr ___length(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("Invalid 'length' statement");
			return nullptr;
		}

		auto arg = eval(ctx, root->get(1));

		node_ptr result(new node);
		result->type = node_type::integer;
		result->value = std::to_string(arg->length());

		return result;
	}

	node_ptr ___empty(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("Invalid 'empty?' statement");
			return nullptr;
		}

		auto arg = eval(ctx, root->get(1));
        bool is_empty = (arg == nullptr)                ||
                        (arg->car == nullptr)           ||
                        (arg->type == node_type::empty) ||
                        (arg->length() == 0);

		node_ptr result(new node);
		result->type = node_type::boolean;
		result->value = is_empty ? "true" : "false";

		return result;
	}

	node_ptr ___print(context& ctx, const node_ptr& root)
	{
		if (root->length() > 1)
		{
			output("", eval(ctx, root->cdr->car));			
		}
		return nullptr;
	}

	node_ptr ___println(context& ctx, const node_ptr& root)
	{
		if (root->length() > 1)
		{
			outputln("", eval(ctx, root->cdr->car));			
		}
		return nullptr;
	}

	node_ptr ___add(context& ctx, const node_ptr& root)
	{		
		if (root->length() != 3)
		{
			log_errorln("'___add' expects two arguments");
			return nullptr;
		}

		node_ptr first_val = eval(ctx, root->get(1));
		log_traceln("___add first arg: ", first_val);

		if (first_val == nullptr)
		{
			log_errorln("___add: first arg is null");
			return nullptr;
		}

		node_ptr second_val = eval(ctx, root->get(2));
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
			result->value = std::to_string(first_val->to_int() + second_val->to_int());
		}
		else 
		{
			result->value = std::to_string(first_val->to_float() + second_val->to_float());
		}

		return result;
	}

	node_ptr ___mult(context& ctx, const node_ptr& root)
	{
		return root;
	}
}