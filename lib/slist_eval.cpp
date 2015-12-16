#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <istream>

namespace
{
    slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
    slist::node_ptr eval_name(slist::context& ctx, const slist::node_ptr& root);

    void dump_callstack(slist::context& ctx)
    {
        using namespace slist;
        int index = 0;
        for (auto& item : ctx.callstack)
        {
            log_traceln("[" + std::to_string(index) + "]: ", item.node);
            ++index;
        }
    }
}

namespace slist
{
    node_ptr eval(context& ctx, const node_ptr& root)
    {
        log_traceln("Eval: ", root);
        debug_print_environment(ctx, ctx.active_env);

        if (root == nullptr)
        {
            return nullptr;
        }

        context::callstack_item item;
        item.node = root;

        ctx.callstack.push_back(item);

        dump_callstack(ctx);
        
        node_ptr result;

        switch (root->type)
        {
            case node_type::empty:
                result = root;
                break;
            case node_type::pair:
                result = eval_list(ctx, root);
                break;
            case node_type::name:
                result = eval_name(ctx, root);
                break;
            case node_type::boolean:
            case node_type::integer:
            case node_type::number:
            case node_type::string:
                result = root;
            default:
                break;
        }

        log_trace("Result of ", root);
        log_traceln(" -> ", result);
        debug_print_environment(ctx, ctx.active_env);

        auto& back_item = ctx.callstack.back();
        ctx.callstack.pop_back();
        
        if (back_item.delayed_proc != nullptr)
        {
            log_traceln("TAIL CALL ELIMINATION!!!!");
            
            for (auto& pair : back_item.delayed_proc->env->bindings)
            {
                ctx.active_env->register_variable(pair.first, pair.second);
            }

            back_item.delayed_proc->env = ctx.active_env;
            
            result = eval_procedure(ctx, back_item.delayed_proc, back_item.delayed_args);
        }


        return result;
    }

    node_ptr eval_procedure(context& ctx, const procedure_ptr& f, const node_ptr& args)
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
            if (f->is_tail)
            {
                
                dump_callstack(ctx);

                log_traceln("Trying to unwind the stack:\n", f->body);
                
                // Try to unwind the call stack: tail-call elimination
                int size = static_cast<int>(ctx.callstack.size());
                int i = size-1;
                while (i > 0)
                {
                    //auto& item = ctx.callstack[i];
                    auto& prev_item = ctx.callstack[i-1];
                    log_traceln("    Prev: ", prev_item.node);
                    if (prev_item.node->is_tail)
                    {
                        if (prev_item.node == f->body)
                        {
                            prev_item.delayed_proc = f;
                            prev_item.delayed_args = args;
                            return nullptr;
                        }
                        --i;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            
            log_traceln("Executing procedure:\n", f->body);

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

                    if (proc->is_macro)
                    {
                        // Do not evaluate macro arguments
                        env->register_variable(var_name->value, arg);
                    }
                    else 
                    {
                        node_ptr list_arg(std::make_shared<node>());
                        list_arg->type = node_type::pair;
                        while (arg)
                        {
                            list_arg->append(eval(ctx, arg->car));
                            arg = arg->cdr;
                        }

                        env->register_variable(var_name->value, list_arg);
                    }
                    break;
                }

                if (proc->is_macro)
                {
                    // Do not evaluate macro arguments
                    env->register_variable(var_name->value, arg->car);
                }
                else 
                {
                    env->register_variable(var_name->value, eval(ctx, arg->car));                   
                }

                arg = arg->cdr;
                var = var->cdr;
            }
        }

        // log_traceln("Evaluating Procedure from 'apply':\n", nullptr, proc);

        if (proc->is_macro)
        {
            // log_traceln("Macro root: ", nullptr, proc);
            node_ptr res = eval_procedure(ctx, proc, args);
            // log_traceln("Macro res: ", res);
            return eval(ctx, res);
        }
        else 
        {
            return eval_procedure(ctx, proc, args); 
        }
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
        if (op_node == nullptr)
        {
            log_errorln("Cannot evaluate empty list.");
            return nullptr;
        }
        
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
            // Look in environment
            node_ptr val = ctx.active_env->lookup_variable(op_node->value);
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