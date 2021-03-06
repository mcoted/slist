#include "slist_context.h"
#include "slist_eval.h"
#include "slist_native.h"
#include "slist_log.h"
#include <algorithm>

namespace slist
{
    context::context()
    {
        // Prepare global environment
        global_env = std::make_shared<environment>();
        global_env->is_global = true;

        active_env = global_env;

        register_native("eval",    &native_eval);
        register_native("apply",   &native_apply);
        register_native("cons",    &native_cons);
        register_native("list",    &native_list);
        register_native("car",     &native_car);
        register_native("cdr",     &native_cdr);
        register_native("quote",   &native_quote);
        register_native("unquote", &native_unquote);
        register_native("'",       &native_quote);      
        register_native("lambda",  &native_lambda);
        register_native("define",  &native_define);
        register_native("defmacro",&native_defmacro);
        register_native("set!",    &native_set);
        register_native("let",     &native_let);
        register_native("letrec",  &native_letrec);
        register_native("begin",   &native_begin);
        register_native("if",      &native_if);
        register_native("length",  &native_length);
        register_native("empty?",  &native_empty);
        register_native("print",   &native_print);
        register_native("println", &native_println);
        register_native("eq?",     &native_eq);
        register_native("equal?",  &native_equal);
        register_native("not",     &native_not);

        register_native("pair?",   &native_is_pair);
        register_native("boolean?",&native_is_bool);
        register_native("integer?",&native_is_int);
        register_native("number?", &native_is_number);
        register_native("string?", &native_is_string);
        register_native("symbol?", &native_is_symbol);

        register_native("+",       &native_add);
        register_native("-",       &native_sub);
        register_native("*",       &native_mul);
        register_native("/",       &native_div);
        register_native("%",       &native_mod);

        register_native("=",       &native_e);
        register_native("!=",      &native_ne);
        register_native("<",       &native_lt);
        register_native(">",       &native_gt);
        register_native("<=",      &native_le);
        register_native(">=",      &native_ge);

        register_native("assert", &native_assert);

        // Execute the builtins script to register the builtin procedures
        //exec(*this, builtins);
    }

    void context::register_native(const std::string& name, procedure::callback func)
    {
        procedure_ptr f(std::make_shared<procedure>());
        f->env->parent = active_env;
        f->is_native = true;
        f->native_func = func;

        node_ptr n(std::make_shared<node>());
        n->proc = f;

        global_env->register_variable(name, n);
    }

    node_ptr context::lookup_symbol(const std::string& name)
    {
        auto it = symbols.find(name);
        if (it != symbols.end())
        {
            return it->second;
        }
        return nullptr;
    }

    void context::insert_symbol(const node_ptr& node)
    {
        if (node == nullptr || node->type != node_type::name)
        {
            log_errorln("Trying to insert an invalid symbol: ", node);
            return;
        }

        symbols[node->value] = node;
    }

    void context::debug_dump_callstack()
    {
        using namespace slist;
        int index = 0;
        for (auto& item : callstack)
        {
            (void)item;
            LOG_TRACELN2("[" + std::to_string(index) + "]: ", item.node);
            ++index;
        }
    }
}