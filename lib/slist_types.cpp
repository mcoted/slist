#include "slist_types.h"
#include "slist_log.h"
#include "slist_context.h"
#include <algorithm>

namespace slist
{
	node::node()
		: type(node_type::empty)
	{
	}

	size_t node::length() const
	{
		size_t len = 0;
		const node *p = this;
		while (p != nullptr)
		{
			p = p->cdr.get();
			++len;
		}
		return len;
	}

	node_ptr node::get(size_t index)
	{
		node_ptr p = shared_from_this();
		while (index > 0 && p != nullptr)
		{
			p = p->cdr;
			--index;
		}
		return p != nullptr ? p->car : p;
	}

	void node::append(const node_ptr& n)
	{
		node *p = this;

        if (p->car == nullptr)
        {
            p->car = n;
            return;
        }
        
        while (p->cdr != nullptr)
		{
			if (p->car == nullptr)
			{
				p->car = n;
				return;
			}
			p = p->cdr.get();
		}

		node_ptr next(std::make_shared<node>());
        next->type = node_type::pair;
		next->car = n;
		p->cdr = next;
	}

	node_ptr node::pop()
	{
		if (cdr == nullptr)
		{
			return nullptr;
		}

		node_ptr n =shared_from_this();
		while (n->cdr != nullptr)
		{
			n = n->cdr;
		}

		node_ptr result = n->cdr;
		n->cdr = nullptr;

		return result;
	}

	bool node::to_bool() const 
	{
		if (type != node_type::boolean)
		{
			log_errorln(std::string("Cannot convert to boolean, invalid type: ") + std::to_string((int)type));
			return false;
		}
		std::string v = value;
		std::transform(v.begin(), v.end(), v.begin(), ::tolower);
		return v == "true";
	}

	void node::set_bool(bool v)
	{
		type = node_type::boolean;
		value = v ? "true" : "false";
	}

	int node::to_int() const 
	{
		if (type != node_type::integer) 
		{
			log_errorln(std::string("Cannot convert to int, invalid type: ") + std::to_string((int)type));
			return 0;
		}
		return std::stoi(value);
	}

	void node::set_int(int v)
	{
		type = node_type::integer;
		value = std::to_string(v);
	}

	float node::to_float() const
	{
		if (type != node_type::integer && 
			type != node_type::number)
		{
			log_errorln(std::string("Cannot convert to float, invalid type: ") + std::to_string((int)type));

			return 0;			
		}
		return std::stof(value);
	}

	void node::set_float(float v)
	{
		type = node_type::number;
		value = std::to_string(v);
	}

	const std::string& node::to_name() const
	{
		return value;
	}

	void node::set_name(const std::string& str)
	{
		type = node_type::name;
		value = str;
	}

	const std::string& node::to_string() const
	{
		return value;
	}

	void node::set_string(const std::string& str)
	{
		type = node_type::string;
		value = str;
	}

	procedure::procedure()
		 : is_native(false)
		 , is_macro(false)
	 {
	 	env = std::make_shared<environment>();
	 }

	void environment::register_variable(const std::string& name, node_ptr n)
	{
		bindings[name] = n;
	}

	node_ptr environment::lookup_variable(const std::string& name)
	{
		auto it = bindings.find(name);
		if (it != bindings.end())
		{
			return it->second;
		}

		auto p = parent;
		if (p != nullptr)
		{
			return p->lookup_variable(name);
		}

		return nullptr;
	}

	bool environment::set_variable(const std::string& name, node_ptr n)
	{
		environment *env = this;
		while (env != nullptr)
		{
			auto it = env->bindings.find(name);
			if (it != env->bindings.end())
			{
				it->second = n;
				return true;
			}

			env = env->parent.get();
		}
		return false;
	}

	void print_node(const node_ptr& n)
	{
		debug_print_node(n);
	}

	void debug_print_node(const node_ptr& n, int indent)
	{
		for (int i = 0; i < indent; ++i)
		{
			log_trace("    ");
		}

		if (n == nullptr)
		{
			log_traceln("nil");
            return;
		}

		log_trace("[" + type_to_string(n->type) + "]");		

		if (n->value.length() > 0)
		{
			log_trace(" \"" + n->value + "\"");
		}

		log_traceln();

		if (n->type == node_type::pair)
		{
			debug_print_node(n->car, indent+1);
			debug_print_node(n->cdr, indent+1);
		}
	}

	void debug_print_environment(context& ctx, const environment_ptr& env)
	{
		if (env == nullptr)
		{
			return;
		}
		else if (env == ctx.global_env)
		{
			log_traceln("<globals>");
			return;
		}

		log_trace("[");
		for (auto& keyval : env->bindings)
		{
			log_trace(keyval.first + ": ");
            if (keyval.second == nullptr)
            {
                log_trace("<null>");
                continue;
            }
			auto proc = keyval.second->proc;
			if (proc != nullptr)
			{
				if (proc->is_native)
				{
					log_trace("<native func>");
				}
				else 
				{
					log_trace("", proc->body);
				}
			}
			else 
			{
				log_trace(keyval.second->value);
			}
			log_trace(", ");
		}
		log_trace("]");
		debug_print_environment(ctx, env->parent);
	}

	std::string type_to_string(slist::node_type type)
	{
		using namespace slist;

		switch (type)
		{
			case node_type::empty:   return "empty";
			case node_type::pair:    return "pair";
			case node_type::boolean: return "boolean";
			case node_type::integer: return "integer";
			case node_type::number:  return "number";
			case node_type::name:    return "name";
			case node_type::string:  return "string";
		}

		return "<undefined>";
	}
}