#include "slist_native.h"
#include "slist_context.h"
#include "slist_eval.h"
#include "slist_log.h"
#include <algorithm>
#include <stdexcept>

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
			name_node->type = node_type::name;			
			lambda_node->append(name_node);
			lambda_node->append(args);
			lambda_node->append(body);

			log_traceln("LAMBDA FROM SCRATCH:\n", lambda_node);

            ctx.global_env->register_variable(name->value, ___lambda(ctx, lambda_node));
		}
		else if (first->type == node_type::name)
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

		funcdef_ptr func(new funcdef);
		func->env->parent = ctx.active_env;
		func->is_native = false;
		func->name = root->get(0)->value; // "lambda"
		func->variables = root->get(1);
		func->body = root->get(2);

		node_ptr res(new node);
		res->proc = func;

		log_traceln("Lambda proc:\n", nullptr, func);

		return res;
	}

	node_ptr ___eval(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("'eval' expects one argument: ", root);
			return nullptr;
		}

		return eval(ctx, eval(ctx, root->get(1)));
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
        return root->cdr;
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

	node_ptr ___quote_arg(context& ctx, node_ptr arg)
	{
		node_ptr result;
		if (arg->type == node_type::name)
		{
			auto symb = ctx.lookup_symbol(arg->value);
			if (symb != nullptr)
			{
				result = symb;
			}
			else 
			{
				ctx.insert_symbol(arg);
				result = arg;
			}
		}
		else if (arg->type == node_type::integer || arg->type == node_type::number)
		{
			return arg;
		}
		else if (arg->type == node_type::pair)
		{
			result.reset(new node);
			result->type = node_type::pair;

			while (arg != nullptr)
			{
				if (arg->car == nullptr)
				{
					continue;
				}
				node_ptr n = ___quote_arg(ctx, arg->car);
				if (n != nullptr)
				{
					result->append(n);
				}
				else 
				{
					log_errorln("Cannot quote: ", arg->car);
					return nullptr;
				}
				
				arg = arg->cdr;
			}
		}
		else 
		{
			log_errorln("Invalid argument to quote: ", arg);
			return nullptr;
		}
		
		return result;
	}

	node_ptr ___quote(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("'quote' expects one argument:\n", root);
			return nullptr;
		}

		node_ptr arg = root->get(1);

		return ___quote_arg(ctx, arg);
	}

	node_ptr ___let(context& ctx, const node_ptr& root)
	{
		// Let is syntactic sugar:
		//    (lambda (x)
		//        (let ((y 2)) (+ x y))) 
		// <-->
		//    (lambda (x)
		//        ((lambda (y) (+ x y)) 1))

		if (root->length() != 3)
		{
			log_errorln("Invalid 'let' syntax: ", root);
			return nullptr;
		}

		environment_ptr env(new environment);
		env->parent = ctx.active_env;

		node_ptr bindings = root->get(1);
		if (bindings == nullptr || bindings->type != node_type::pair)
		{
			log_errorln("Invalid bindings for 'let': ", root);
			return nullptr;
		}

		node_ptr binding = bindings;
		while (binding != nullptr)
		{
			node_ptr name_value = binding->car;
			if (name_value == nullptr || name_value->length() != 2)
			{
				log_errorln("Invalid 'let' binding: ", binding);
				return nullptr;
			}

			node_ptr var_name = name_value->get(0);
            if (var_name->type != node_type::name)
            {
                log_errorln("Invalid variable name in binding: ", name_value);
                return nullptr;
            }

            node_ptr value = eval(ctx, name_value->get(1));
			env->register_variable(var_name->value, value);

			binding = binding->cdr;
		}

		auto old_active_env = ctx.active_env;
		ctx.active_env = env;

		funcdef_ptr func(new funcdef);
		func->env = env;
		func->is_native = false;
		func->name = root->get(0)->value; // "let"
		func->body = eval(ctx, root->get(2)); // Need to eval here

		ctx.active_env = old_active_env;

		node_ptr res(new node);
		res->proc = func;

		log_traceln("'let' proc:\n", nullptr, func);

		return eval(ctx, func, nullptr);	
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
			log_errorln("Predicate did not evaluate to a boolean value");
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

		// TODO: I need a more 'standard' way to define an empty node
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
			output("", eval(ctx, root->cdr->car), nullptr, true);
		}
		return nullptr;
	}

	node_ptr ___println(context& ctx, const node_ptr& root)
	{
		if (root->length() > 1)
		{
			outputln("", eval(ctx, root->cdr->car), nullptr, true);
		}
		return nullptr;
	}

	node_ptr ___eq(context& ctx, const node_ptr& root)
	{
		if (root->length() != 3)
		{
			log_errorln("'eq?' requires 2 arguments: ", root);
			return nullptr;
		}

		node_ptr v1 = eval(ctx, root->get(1));
		node_ptr v2 = eval(ctx, root->get(2));

		bool value = false;
		if (v1->type == v2->type)
		{
			if (v1->type == node_type::integer || v1->type == node_type::number)
			{
				value = v1->value == v2->value;
			}
			else 
			{
				value = (v1 == v2);
			}
		}

		node_ptr result(new node);
		result->type = node_type::boolean;
		result->value = value ? "true" : "false";

		return result;
	}

	bool ___equal_helper(context& ctx, const node_ptr& arg1, const node_ptr& arg2)
	{
		if (arg1 == nullptr && arg2 == nullptr)
		{
			return true;
		}
		else if (arg1 == nullptr || arg2 == nullptr)
		{
			return false;
		}
		else 
		{
			bool result = false;
			if (arg1->type == arg2->type)
			{
				if (arg1->type == node_type::integer || 
					arg1->type == node_type::number  ||
					arg1->type == node_type::string)
				{
					result = (arg1->value == arg2->value);
				}
				else if (arg1->type == node_type::pair)
				{
					result = ___equal_helper(ctx, arg1->car, arg2->car) &&
					         ___equal_helper(ctx, arg1->cdr, arg2->cdr);
				}
			}
			return result;
		}
	}

	node_ptr ___equal(context& ctx, const node_ptr& root)
	{
		if (root->length() != 3)
		{
			log_errorln("'eq?' requires 2 arguments: ", root);
			return nullptr;
		}

		node_ptr arg1 = eval(ctx, root->get(1));
		node_ptr arg2 = eval(ctx, root->get(2));

		bool value = ___equal_helper(ctx, arg1, arg2);

		node_ptr result(new node);
		result->type = node_type::boolean;
		result->value = value ? "true" : "false";

		return result;
	}

	node_ptr ___not(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("'not' requires 1 argument: ", root);
			return nullptr;
		}

		node_ptr arg = eval(ctx, root->get(1));

		if (arg->type != node_type::boolean)
		{
			log_errorln("'not' argument did not evaluate to a boolean value: ", arg);
			return nullptr;
		}

		node_ptr result(new node);
		result->type = node_type::boolean;
		result->value = arg->value == "true" ? "false" : "true";

		return result;
	}

	bool ___arithmetic_op_validate_arg(const node_ptr& arg)
	{
        if (arg == nullptr)
        {
            log_errorln("Invalid argument to arithmetic operator: nullptr");
            return false;
        }
		if (arg->type != node_type::integer && arg->type != node_type::number)
		{
			log_errorln("Invalid argument to arithmetic operator:", arg);
			return false;
		}
		return true;
	}

	template<class Op>
	node_ptr ___arithmetic_op_helper(const node_ptr& n, const node_ptr& arg, const Op& op)
	{
		if (!___arithmetic_op_validate_arg(arg))
		{
			return nullptr;
		}

        node_ptr result(new node);
		if (n->type == node_type::number || arg->type == node_type::number)
		{
			result->type = node_type::number;
			result->value = op.perform_float(n->value, arg->value);
		}
		else 
		{
			result->type = node_type::integer;
			result->value = op.perform_int(n->value, arg->value);
		}
		return result;
	}

	#define MAKE_ARITHMETIC_OP(OP) \
		struct OperatorWrapper \
		{ \
			std::string perform_int(const std::string& arg1, const std::string& arg2) const \
			{ \
				using namespace std; \
				return to_string(stoi(arg1) OP stoi(arg2)); \
			} \
			std::string perform_float(const std::string& arg1, const std::string& arg2) const \
			{ \
				using namespace std; \
				return to_string(stof(arg1) OP stof(arg2)); \
			} \
		} op;

	#define MAKE_ARITHMETIC_FUNC(FUNC_NAME, OP) \
		node_ptr FUNC_NAME(context& ctx, const node_ptr& root) \
		{ \
			if (root->length() < 2) \
			{ \
				log_errorln("'" #OP "' expects at least one argument"); \
				return nullptr; \
			} \
			\
			MAKE_ARITHMETIC_OP(OP); \
			\
			node_ptr arg = root->cdr; \
			\
			node_ptr result = eval(ctx, arg->car); \
			\
			if (!___arithmetic_op_validate_arg(result)) \
			{ \
				return nullptr; \
			} \
			\
			arg = arg->cdr; \
			while (arg != nullptr) \
			{ \
				result = ___arithmetic_op_helper(result, eval(ctx, arg->car), op); \
				if (result == nullptr) \
				{ \
					return nullptr; \
				} \
				arg = arg->cdr; \
			} \
			\
			return result; \
		}

	MAKE_ARITHMETIC_FUNC(___add, +)
	MAKE_ARITHMETIC_FUNC(___sub, -)
	MAKE_ARITHMETIC_FUNC(___mul, *)
	MAKE_ARITHMETIC_FUNC(___div, /)

	#define MAKE_COMPARISON_OP_FUNC(FUNC_NAME, OP) \
		node_ptr FUNC_NAME(context& ctx, const node_ptr& root) \
		{ \
			if (root->length() < 3) \
			{ \
				log_errorln("'" #OP "' expects 2 numeric arguments"); \
				return nullptr; \
			} \
			\
			node_ptr a1 = eval(ctx, root->get(1)); \
			node_ptr a2 = eval(ctx, root->get(2)); \
			\
			if ((a1 == nullptr || (a1->type != node_type::integer && a1->type != node_type::number)) || \
				(a2 == nullptr || (a2->type != node_type::integer && a2->type != node_type::number))) \
			{ \
				log_errorln("'" #OP "' expects 2 numeric arguments"); \
				return nullptr; \
			} \
			\
			bool greater = std::stof(a1->value) OP std::stof(a2->value); \
			\
			node_ptr result(new node); \
			result->type = node_type::boolean; \
			result->value = greater ? "true" : "false"; \
			\
			return result; \
		}

	MAKE_COMPARISON_OP_FUNC(___e,  ==)
	MAKE_COMPARISON_OP_FUNC(___ne, !=)
	MAKE_COMPARISON_OP_FUNC(___lt, < )
	MAKE_COMPARISON_OP_FUNC(___gt, > )
	MAKE_COMPARISON_OP_FUNC(___le, <=)
	MAKE_COMPARISON_OP_FUNC(___ge, >=)

	node_ptr ___assert(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("'assert' requires 1 argument: ", root);
			return nullptr;
		}

		node_ptr arg = eval(ctx, root->get(1));

		if (arg->type != node_type::boolean)
		{
			log_errorln("'assert' argument did not evaluate to a boolean value: ", arg);
			return nullptr;
		}

		if (arg->value != "true")
		{
			throw std::runtime_error("assert failure!");
		}

		return nullptr;
	}

	node_ptr ___run_test(context& ctx, const node_ptr& root)
	{
		if (root->length() != 2)
		{
			log_errorln("'___run_test' expects 1 predicate argument: ", root);
			return nullptr;
		}

		node_ptr raw_arg = root->get(1);
		node_ptr arg = eval(ctx, raw_arg);

		if (arg->type != node_type::boolean)
		{
			log_errorln("'___run_test' argument did not evaluate to a boolean value: ", arg);
			return nullptr;
		}

		output("Testing: ", raw_arg);

		if (arg->value != "true")
		{
			outputln("      FAILED");
		}
		else 
		{
			outputln("      OK");
		}
		return nullptr;

	}
}