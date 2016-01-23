#include "slist_native.h"
#include "slist_context.h"
#include "slist_eval.h"
#include "slist_log.h"

#include <algorithm>
#include <stdexcept>

namespace slist
{
    node_ptr native_eval(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("'eval' expects one argument: ", root);
            return nullptr;
        }

        return eval(ctx, eval(ctx, root->get(1)));
    }

    node_ptr native_apply(context& ctx, const node_ptr& root)
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

        return apply(ctx, args, func_node);
    }

    node_ptr native_cons(context& ctx, const node_ptr& root)
    {
        if (root->length() != 3)
        {
            log_errorln("Invalid arguments to 'cons':\n", root);
            return nullptr;
        }

        node_ptr result(std::make_shared<node>());
        result->type = node_type::pair;
        result->car = eval(ctx, root->get(1));
        result->cdr = eval(ctx, root->get(2));

        return result;
    }

    node_ptr native_list(context& ctx, const node_ptr& root)
    {
        return root->cdr;
    }

    node_ptr native_car(context& ctx, const node_ptr& root)
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

    node_ptr native_cdr(context& ctx, const node_ptr& root)
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
        
        node_ptr empty(std::make_shared<node>());
        empty->type = node_type::pair;

        return empty;
    }

    node_ptr native_quote_arg(context& ctx, node_ptr arg)
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
        else if (arg->type == node_type::integer || 
                 arg->type == node_type::number  ||
                 arg->type == node_type::string)
        {
            return arg;
        }
        else if (arg->type == node_type::pair)
        {
            if (arg->length() == 2)
            {
                // Check if it's an 'unquote'
                node_ptr subarg = arg->get(0);
                if (subarg->type == node_type::name &&
                    (subarg->value == "unquote"))
                {
                    return eval(ctx, arg->get(1));
                }
            }

            result = std::make_shared<node>();
            result->type = node_type::pair;

            while (arg != nullptr)
            {
                if (arg->car == nullptr)
                {
                    arg = arg->cdr;
                    continue;
                }

                node_ptr n = native_quote_arg(ctx, arg->car);
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

    node_ptr native_quote(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("'quote' expects one argument:\n", root);
            return nullptr;
        }

        node_ptr arg = root->get(1);

        return native_quote_arg(ctx, arg);
    }

    node_ptr native_unquote(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("'unquote' expects one argument:\n", root);
            return nullptr;
        }

        return root->get(1);
    }

    node_ptr native_lambda(context& ctx, const node_ptr& root)
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

        procedure_ptr func(std::make_shared<procedure>());
        func->env->parent = ctx.active_env;
        func->is_native = false;
        func->name = root->get(0)->value; // "lambda"
        func->variables = root->get(1);
        func->body = root->get(2);

        node_ptr res(std::make_shared<node>());
        res->proc = func;
        res->set_as_tail();

        // LOG_TRACELN("Lambda proc:\n", nullptr, func);

        return res;        
    }

    node_ptr make_define(context& ctx, const node_ptr& root, bool is_macro)
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

            node_ptr lambda_node(std::make_shared<node>());
            lambda_node->type = node_type::pair;

            node_ptr name_node(std::make_shared<node>());
            name_node->value = "lambda";
            name_node->type = node_type::name;          
            lambda_node->append(name_node);
            lambda_node->append(args);
            lambda_node->append(body);

            // LOG_TRACELN("Lambda from scratch:\n", lambda_node);

            node_ptr lambda = native_lambda(ctx, lambda_node);

            if (lambda != nullptr && lambda->proc != nullptr)
            {
                lambda->proc->is_macro = is_macro;
            }

            ctx.active_env->register_variable(name->value, lambda);
        }
        else if (first->type == node_type::name)
        {
            node_ptr res = eval(ctx, body);
            ctx.active_env->register_variable(first->value, res);            
        }

        return nullptr;
    }

    node_ptr native_define(context& ctx, const node_ptr& root)
    {
        return make_define(ctx, root, false);
    }

    node_ptr native_defmacro(context& ctx, const node_ptr& root)
    {
        return make_define(ctx, root, true);
    }

    node_ptr native_set(context& ctx, const node_ptr& root)
    {
        if (root->length() != 3)
        {
            log_errorln("Invalid 'set!' syntax: ", root);
            return nullptr;
        }

        node_ptr name = root->get(1);
        node_ptr arg = eval(ctx, root->get(2));

        if (!ctx.active_env->set_variable(name->value, arg))
        {
            log_errorln("Cannot set unbound variable: ", name);
            return nullptr;
        }

        return nullptr;
    }

    node_ptr native_let(context& ctx, const node_ptr& root)
    {
        // Let is syntactic sugar:
        //    (lambda (x)
        //        (let ((y 2)) (+ x y))) 
        // <-->
        //    (lambda (x)
        //        ((lambda (y) (+ x y)) 2))

        if (root->length() != 3)
        {
            log_errorln("Invalid 'let' syntax: ", root);
            return nullptr;
        }

        environment_ptr env(std::make_shared<environment>());
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

        procedure_ptr func(std::make_shared<procedure>());
        func->is_native = false;
        func->name = root->get(0)->value; // "let"
        func->env = env;
        func->body = root->get(2);

        ctx.active_env = old_active_env;

        node_ptr res(std::make_shared<node>());
        res->proc = func;

        // LOG_TRACELN("'let' proc:\n", nullptr, func);

        return eval_procedure(ctx, res, nullptr);
    }

    node_ptr native_letrec(context& ctx, const node_ptr& root)
    {
        // Example:
        //    (lambda (x)
        //        (letrec ((y 2)) (+ x y))) 

        // Letrec is equivalent to:
        //    (lambda (x)
        //        ((lambda (y)
        //            (begin
        //                (set! y 2) ; <-- argument is transfered here
        //                           ;     and becomes part of closure's environment
        //                (+ x y))) '()))

        if (root->length() != 3)
        {
            log_errorln("Invalid 'letrec' syntax: ", root);
            return nullptr;
        }

        environment_ptr env(std::make_shared<environment>());
        env->parent = ctx.active_env;

        node_ptr bindings = root->get(1);
        if (bindings == nullptr || bindings->type != node_type::pair)
        {
            log_errorln("Invalid bindings for 'letrec': ", root);
            return nullptr;
        }

        // Build a (begin) node to store the values
        node_ptr begin_node(std::make_shared<node>());
        begin_node->type = node_type::pair;

        node_ptr begin_name(std::make_shared<node>());
        begin_name->set_name("begin");
        begin_node->append(begin_name);

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

            environment_ptr temp_env = ctx.active_env;
            ctx.active_env = env;

            node_ptr set_node(std::make_shared<node>());
            set_node->type = node_type::pair;

            node_ptr set_name(std::make_shared<node>());
            set_name->set_name("set!");

            set_node->append(set_name);
            set_node->append(var_name);
            set_node->append(eval(ctx, name_value->get(1)));

            begin_node->append(set_node);

            env->register_variable(var_name->to_name(), std::make_shared<node>());

            ctx.active_env = temp_env;

            binding = binding->cdr;
        }

        begin_node->append(root->get(2));

        auto old_active_env = ctx.active_env;
        ctx.active_env = env;

        procedure_ptr func(std::make_shared<procedure>());
        func->is_native = false;
        func->name = root->get(0)->value; // "let"
        func->env = env;
        func->body = begin_node;

        ctx.active_env = old_active_env;

        node_ptr res(std::make_shared<node>());
        res->proc = func;

        // LOG_TRACELN("'letrec' proc:\n", nullptr, func);

        return eval_procedure(ctx, res, nullptr);
    }

    node_ptr native_begin(context& ctx, const node_ptr& root)
    {
        node_ptr n = root->cdr;

        node_ptr result;
        bool stop = false;
        while (!stop)
        {
            if (n->cdr == nullptr)
            {
                if (root->is_tail)
                {
                    n->car->set_as_tail();
                }
                stop = true;
            }
            result = eval(ctx, n->car);
            n = n->cdr;
        }

        return result;
    }

    node_ptr native_if(context& ctx, const node_ptr& root)
    {
        if (root->length() != 4)
        {
            log_errorln("Invalid 'if' statement");
            return nullptr;
        }

        auto true_node = root->get(2);
        auto false_node = root->get(3);

        if (root->is_tail)
        {
            true_node->set_as_tail();
            false_node->set_as_tail();
        }

        auto pred = eval(ctx, root->get(1));
        if (pred == nullptr || pred->type != node_type::boolean)
        {
            log_errorln("Predicate did not evaluate to a boolean value");
            return nullptr;
        }

        if (pred->to_bool())
        {
            return eval(ctx, true_node);
        }

        return eval(ctx, false_node);
    }

    node_ptr native_length(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("Invalid 'length' statement");
            return nullptr;
        }

        auto arg = eval(ctx, root->get(1));

        node_ptr result(std::make_shared<node>());
        result->set_int(arg->length());

        return result;
    }

    node_ptr native_empty(context& ctx, const node_ptr& root)
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

        node_ptr result(std::make_shared<node>());
        result->set_bool(is_empty);

        return result;
    }

    node_ptr native_print(context& ctx, const node_ptr& root)
    {
        if (root->length() > 1)
        {
            output("", eval(ctx, root->cdr->car), nullptr, true);
        }
        return nullptr;
    }

    node_ptr native_println(context& ctx, const node_ptr& root)
    {
        if (root->length() > 1)
        {
            outputln("", eval(ctx, root->cdr->car), nullptr, true);
        }
        return nullptr;
    }

    node_ptr native_eq(context& ctx, const node_ptr& root)
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

        node_ptr result(std::make_shared<node>());
        result->set_bool(value);

        return result;
    }

    bool native_equal_helper(context& ctx, const node_ptr& arg1, const node_ptr& arg2)
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
                    result = native_equal_helper(ctx, arg1->car, arg2->car) &&
                             native_equal_helper(ctx, arg1->cdr, arg2->cdr);
                }
            }
            return result;
        }
    }

    node_ptr native_equal(context& ctx, const node_ptr& root)
    {
        if (root->length() != 3)
        {
            log_errorln("'eq?' requires 2 arguments: ", root);
            return nullptr;
        }

        node_ptr arg1 = eval(ctx, root->get(1));
        node_ptr arg2 = eval(ctx, root->get(2));

        bool value = native_equal_helper(ctx, arg1, arg2);

        node_ptr result(std::make_shared<node>());
        result->set_bool(value);

        return result;
    }

    node_ptr native_not(context& ctx, const node_ptr& root)
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

        node_ptr result(std::make_shared<node>());
        result->set_bool(!arg->to_bool());

        return result;
    }

    #define MAKE_PREDICATE_FUNC(FUNCNAME, TYPE) \
        node_ptr FUNCNAME(context& ctx, const node_ptr& root) \
        { \
            if (root->length() != 2) \
            { \
                log_errorln("'" #TYPE "?' expects one argument: ", root); \
                return nullptr; \
            } \
            \
            node_ptr arg = eval(ctx, root->get(1)); \
            \
            node_ptr result(std::make_shared<node>()); \
            result->set_bool(arg->type == node_type::TYPE); \
            \
            return result; \
        }

    MAKE_PREDICATE_FUNC(native_is_pair,    pair)
    MAKE_PREDICATE_FUNC(native_is_bool,    boolean)
    MAKE_PREDICATE_FUNC(native_is_int,     integer)
    MAKE_PREDICATE_FUNC(native_is_number,  number)
    MAKE_PREDICATE_FUNC(native_is_string,  string)

    node_ptr native_is_symbol(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("'symbol?' expects one argument: ", root);
            return nullptr;
        }

        node_ptr arg = eval(ctx, root->get(1)); 

        bool is_symbol = false;
        if (arg->type == node_type::name)
        {
            if (ctx.symbols.find(arg->value) != ctx.symbols.end())
            {
                is_symbol = true;
            }
        }

        node_ptr result(std::make_shared<node>());
        result->set_bool(is_symbol);
        
        return result;
    }

    bool native_arithmetic_op_validate_arg(const node_ptr& arg)
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
    node_ptr native_arithmetic_op_helper(const node_ptr& n, const node_ptr& arg, const Op& op)
    {
        if (!native_arithmetic_op_validate_arg(arg))
        {
            return nullptr;
        }

        node_ptr result(std::make_shared<node>());
        if (n->type == node_type::number || arg->type == node_type::number)
        {
            result->set_float(op.perform_float(n->value, arg->value));
        }
        else 
        {
            result->set_int(op.perform_int(n->value, arg->value));
        }
        return result;
    }

    #define MAKE_ARITHMETIC_OP(OP) \
        struct OperatorWrapper \
        { \
            int perform_int(const std::string& arg1, const std::string& arg2) const \
            { \
                using namespace std; \
                return OP<int>()(stoi(arg1), stoi(arg2)); \
            } \
            float perform_float(const std::string& arg1, const std::string& arg2) const \
            { \
                using namespace std; \
                return OP<float>()(stof(arg1), stof(arg2)); \
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
            if (!native_arithmetic_op_validate_arg(result)) \
            { \
                return nullptr; \
            } \
            \
            arg = arg->cdr; \
            while (arg != nullptr) \
            { \
                result = native_arithmetic_op_helper(result, eval(ctx, arg->car), op); \
                if (result == nullptr) \
                { \
                    return nullptr; \
                } \
                arg = arg->cdr; \
            } \
            \
            return result; \
        }

    template<typename T>
    struct ArithmeticAdd
    {
        T operator()(T first, T second)
        {
            return first + second;
        }
    };

    template<typename T>
    struct ArithmeticSub
    {
        T operator()(T first, T second)
        {
            return first - second;
        }
    };

    template<typename T>
    struct ArithmeticMul
    {
        T operator()(T first, T second)
        {
            return first * second;
        }
    };

    template<typename T>
    struct ArithmeticDiv
    {
        T operator()(T first, T second)
        {
            return first / second;
        }
    };

    template<typename T>
    struct ArithmeticMod
    {        
    };

   template<>
    struct ArithmeticMod<int>
    {
        int operator()(int first, int second)
        {
            return first % second;
        }
    };

   template<>
    struct ArithmeticMod<float>
    {
        float operator()(float first, float second)
        {
            return fmodf(first, second);
        }
    };

    MAKE_ARITHMETIC_FUNC(native_add, ArithmeticAdd)
    MAKE_ARITHMETIC_FUNC(native_sub, ArithmeticSub)
    MAKE_ARITHMETIC_FUNC(native_mul, ArithmeticMul)
    MAKE_ARITHMETIC_FUNC(native_div, ArithmeticDiv)
    MAKE_ARITHMETIC_FUNC(native_mod, ArithmeticMod)

    #define MAKE_COMPARISON_OP_FUNC(FUNC_NAME, OP) \
        node_ptr FUNC_NAME(context& ctx, const node_ptr& root) \
        { \
            if (root->length() < 3) \
            { \
                log_errorln("'" #OP "' expects arguments"); \
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
            bool value = std::stof(a1->value) OP std::stof(a2->value); \
            \
            node_ptr result(std::make_shared<node>()); \
            result->set_bool(value); \
            \
            return result; \
        }

    MAKE_COMPARISON_OP_FUNC(native_e,  ==)
    MAKE_COMPARISON_OP_FUNC(native_ne, !=)
    MAKE_COMPARISON_OP_FUNC(native_lt, < )
    MAKE_COMPARISON_OP_FUNC(native_gt, > )
    MAKE_COMPARISON_OP_FUNC(native_le, <=)
    MAKE_COMPARISON_OP_FUNC(native_ge, >=)

    node_ptr native_assert(context& ctx, const node_ptr& root)
    {
        if (root->length() != 2)
        {
            log_errorln("'assert' requires 1 argument: ", root);
            return nullptr;
        }

        node_ptr arg = eval(ctx, root->get(1));

        if (arg == nullptr || arg->type != node_type::boolean)
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
}