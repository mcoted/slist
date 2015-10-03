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
		node_ptr body = root->get(2);

		if (first->type == node_type::pair)
		{
			// Lambda syntactic sugar
			// (define (f x) (...)) -> (define f (lambda (x) (...))

			node_ptr name = first->car;
			node_ptr args = first->cdr;

			node_ptr lambda_node(new node);
			lambda_node->type = node_type::pair;

			node_ptr name_node(new node);
			name_node->value = "lambda";
			name_node->type = node_type::string;			
			lambda_node->append(name_node);
			lambda_node->append(args);
			lambda_node->append(body);

			log_traceln("LAMBDA FROM SCRATCH:\n", lambda_node);

            ctx.global_env->register_variable(name->value, ___lambda(ctx, lambda_node));
		}
		else if (first->type == node_type::string)
		{
            node_ptr res = eval(ctx, body);
			ctx.global_env->register_variable(first->value, res);            
		}

		return nullptr;
	}

	node_ptr ___lambda(context& ctx, const node_ptr& root)
	{
		if (root->proc != nullptr)
		{
			log_warningln("Evaluating a lambda that already has a procedure. Was it evaluated twice?");
			return root;
		}

		if (root->length() != 3)
		{
			log_errorln("Invalid lambda format\n", root);
			return nullptr;
		}

		funcdef::arg_list arg_list;

		// Parse arguments
		node_ptr arg_node = root->get(1);
		node_ptr args = arg_node;
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
		func->env->parent = ctx.active_env;
		func->name = root->get(0)->value; // "lambda"
		func->args = arg_list;
		func->variables = args;
		func->variadic = false; // TODO
		func->body = root->get(2);

		node_ptr res(new node);
		res->proc = func;

		log_traceln("Lambda proc:\n", nullptr, func);

		return res;
	}

	node_ptr ___apply(context& ctx, const node_ptr& root)
	{
		if (root->length() < 3)
		{
			log_errorln("Not enough arguments for 'apply'\n", root);
			return nullptr;
		}

        node_ptr func_node = eval(ctx, root->get(1));
		if (func_node->proc == nullptr)
		{
			log_errorln("First argument for 'apply' is not a procedure:\n", func_node);
			return nullptr;
		}

        node_ptr args = eval(ctx, root->get(2));
        if (args->type != node_type::pair)
        {
        	log_errorln("Arguments is not a list:\n", args);
        	return nullptr;
        }

		return apply(ctx, args, func_node->proc);
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