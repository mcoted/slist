#include "slist_types.h"
#include "slist_log.h"
#include "slist_context.h"
#include <algorithm>

namespace slist
{
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

		node_ptr next(new node);
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

	int node::to_int() const 
	{
		if (type != node_type::integer) 
		{
			log_errorln(std::string("Cannot convert to int, invalid type: ") + std::to_string((int)type));
			return 0;
		}
		return std::stoi(value);
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

	funcdef::funcdef()
		 : is_native(false)
	 {
	 	env.reset(new environment);
	 }

	 environment::environment()
	 {
	 }

	 environment::environment(const environment& other)
	 	: parent(other.parent)
	 	, bindings(other.bindings)
 	{	 		
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

	void debug_print_environemnt(const environment_ptr& env)
	{
		if (env == nullptr)
		{
			return;
		}
		log_traceln("[");
		for (auto& keyval : env->bindings)
		{
			log_trace("\"" + keyval.first + "\": ");
			auto proc = keyval.second->proc;
			if (proc != nullptr)
			{
				if (proc->is_native)
				{
					log_traceln("<native func>");					
				}
				else 
				{
					log_traceln("", proc->body);
				}
			}
			else 
			{
				log_traceln(keyval.second->value);
			}
		}
		log_traceln("]");
		debug_print_environemnt(env->parent);
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
			case node_type::string:  return "string";
			case node_type::symbol:  return "symbol";
		}

		return "<undefined>";
	}
}