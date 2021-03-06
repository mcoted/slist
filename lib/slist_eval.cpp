#include "slist_eval.h"
#include "slist_parser.h"
#include "slist_log.h"

#include <istream>

namespace
{
    slist::node_ptr eval_list(slist::context& ctx, const slist::node_ptr& root);
    slist::node_ptr eval_name(slist::context& ctx, const slist::node_ptr& root);
}

namespace slist
{
    node_ptr eval(context& ctx, const node_ptr& root)
    {
        ctx.debug_dump_callstack();
        LOG_TRACELN2("Eval: ", root);

        if (root == nullptr)
        {
            return nullptr;
        }
        
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

        LOG_TRACELN2("Result of ", root);
        LOG_TRACELN2(" -> ", result);
        debug_print_environment(ctx, ctx.active_env);

        return result;
    }

    node_ptr eval_procedure(context& ctx, const node_ptr& proc_node, const node_ptr& args)
    {
        if (proc_node == nullptr)
        {
            return nullptr;
        }

        auto proc = proc_node->proc;

        if (proc == nullptr)
        {
            return nullptr;
        }

        context::callstack_item item;
        item.node = proc_node;
        
        ctx.callstack.push_back(item);        
        
        struct auto_pop_callstack
        {
            auto_pop_callstack(context& ctx) : ctx(ctx) {}
            ~auto_pop_callstack() { ctx.callstack.pop_back(); }
            
            context& ctx;
        } auto_stack_popper(ctx);
        
        if (proc->is_native)
        {
            // Build a root node
            node_ptr name_node(std::make_shared<node>());
            name_node->type = node_type::name;
            name_node->value = proc->name;

            node_ptr root(std::make_shared<node>());
            root->type = node_type::pair;
            root->car = name_node;
            root->cdr = args;

            auto prev_env = ctx.active_env; 
            ctx.active_env = proc->env;
            auto result = proc->native_func(ctx, root);
            ctx.active_env = prev_env;

            return result;
        }
        else 
        {
            node_ptr result;

            while (proc != nullptr)
            {
                if (proc->is_tail)
                {
                    ctx.debug_dump_callstack();

                    LOG_TRACELN2("Trying to unwind the stack:\n", proc->body);
                    
                    // Try to unwind the call stack: tail-call elimination
                    int size = static_cast<int>(ctx.callstack.size());
                    int i = size-1;
                    while (i > 0)
                    {
                        //auto& item = ctx.callstack[i];
                        auto& prev_item = ctx.callstack[i-1];
                        LOG_TRACELN2("    Prev: ", prev_item.node);
                        if (prev_item.node->is_tail)
                        {
                            if (prev_item.node->proc == proc)
                            {
                                prev_item.delayed_proc = proc;
                                prev_item.delayed_args = args;
                                LOG_TRACELN2("    Unwinding!: ", prev_item.node);
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
                
                LOG_TRACELN2("Executing procedure:\n", proc->body);

                auto prev_env = ctx.active_env;
                ctx.active_env = proc->env;
                result = eval(ctx, proc->body);
                ctx.active_env = prev_env;

                auto prev_proc = proc;
                
                auto& back_item = ctx.callstack.back();
                proc = back_item.delayed_proc;

                if (proc != nullptr)
                {
                    context::callstack_item item;
                    item.node = proc_node;
                    ctx.callstack.pop_back();
                    ctx.callstack.push_back(item);

                    proc->env->parent = prev_env;
                    
                    LOG_TRACELN("STACK OPTIM!!");
                }
            }

            return result;
        }
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

    node_ptr apply(context& ctx, const node_ptr& args, const node_ptr& proc_node)
    {
        auto& proc = proc_node->proc;

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

        // LOG_TRACELN("Evaluating Procedure from 'apply':\n", nullptr, proc);

        if (proc->is_macro)
        {
            // LOG_TRACELN("Macro root: ", nullptr, proc);
            node_ptr res = eval_procedure(ctx, proc_node, args);
            // LOG_TRACELN("Macro res: ", res);
            return eval(ctx, res);
        }
        else 
        {
            return eval_procedure(ctx, proc_node, args); 
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

        node_ptr proc_node = op_node;
        procedure_ptr proc = proc_node->proc;

        if (proc == nullptr && op_node->type == node_type::pair)
        {
            node_ptr eval_node = eval(ctx, op_node);
            if (eval_node == nullptr || eval_node->proc == nullptr)
            {
                log_errorln("Error: first argument is not a procedure", root);
                return nullptr;
            }
            proc_node = eval_node;
            proc = eval_node->proc;
        }
        else
        {
            // Look in environment
            node_ptr val = ctx.active_env->lookup_variable(op_node->value);
            if (val != nullptr && val->proc != nullptr)
            {
                proc_node = val;
                proc_node->is_tail = root->is_tail;
                proc = val->proc;
                if (proc != nullptr && proc->is_native)
                {
                    return proc->native_func(ctx, root);
                }
            }
        }

        if (proc != nullptr)
        {
            return apply(ctx, root->cdr, proc_node);
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