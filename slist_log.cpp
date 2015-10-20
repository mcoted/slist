#include "slist_log.h"
#include <iostream>

namespace
{
	slist::log_level global_log_level = slist::log_level::warning;
	bool global_log_from_print = false;

	void log(const std::string& str, slist::log_level level, bool in_pair = false);
	void log(const slist::node_ptr& n, slist::log_level level, bool in_pair = false);
	void log(const slist::funcdef_ptr& f, slist::log_level level, bool in_pair = false);
	void log_internal(const std::string& str, slist::log_level level);
}

namespace slist
{
	log_level get_log_level()
	{
		return global_log_level;
	}

	void set_log_level(log_level level)
	{
		global_log_level = level;
	}

	void output(const std::string& str, node_ptr n, funcdef_ptr f, bool from_print)
	{
		global_log_from_print = from_print;
		log(str, log_level::always);
		log(n, log_level::always);
		log(f, log_level::always);
		global_log_from_print = false;
	}

	void outputln(const std::string& str, node_ptr n, funcdef_ptr f, bool from_print)
	{
		output(str, n, f, from_print);
		log("\n", log_level::always);
	}

	void log_warning(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log(str, log_level::warning);
		log(n, log_level::warning);
		log(f, log_level::warning);
	}	

	void log_warningln(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log_warning(str, n, f);
		log("\n", log_level::warning);
	}

	void log_error(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log(str, log_level::error);
		log(n, log_level::error);
		log(f, log_level::error);
	}

	void log_errorln(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log_error(str, n, f);
		log("\n", log_level::error);
	}

	void log_trace(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log(str, log_level::trace);
		log(n, log_level::trace);
		log(f, log_level::trace);
	}

	void log_traceln(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log_trace(str, n, f);
		log("\n", log_level::trace);
	}
}

namespace
{
	void log(const std::string& str, slist::log_level level, bool in_pair)
	{
		if (level > global_log_level)
		{
			return;
		}

		log_internal(str, level);
	}

	void log(const slist::node_ptr& n, slist::log_level level, bool in_pair)
	{
		using namespace slist;

		if (level > global_log_level || n == nullptr)
		{
			return;
		}

		if (n == nullptr)
		{
			if (global_log_level >= log_level::trace)
			{
				log_internal("nil\n", level);
			}
			return;
		}

		switch (n->type)
		{
			case slist::node_type::pair:
				if (n->car != nullptr && 
					n->car->type == node_type::name && 
					n->car->value == "'")
				{
					// Quote, skip the list format
					log(n->car, level, false);
					log(n->cdr, level, true);
				}
				else 
				{
					if (!in_pair)
					{
						log_internal("(", level);
					}
					log(n->car, level, false);
					if (n->cdr != nullptr)
					{
						if (n->cdr->type == slist::node_type::pair)
						{
							log_internal(" ", level);
							log(n->cdr, level, true);
						}
						else
						{
							log_internal(" . ", level);
							log(n->cdr, level, true);
						}	
					} 
					if (!in_pair)
					{
						log_internal(")", level);
					}					
				}
				break;
			case node_type::string:
				{
					if (global_log_from_print)
					{
						log_internal(n->value, level);
					}
					else
					{
						log_internal("\"" + n->value + "\"", level);						
					}
				}
				break;
			default:
				if (n->proc != nullptr)
				{
					log_internal("procedure: " + n->proc->name, level);
					if (n->proc->body != nullptr)
					{
						log_internal(" ", level);
						log(n->proc->body, level);
					}
				}
				else 
				{
					log_internal(n->value, level);				
				}
				break;
		}
	}

	void log_env(const slist::environment_ptr& env, slist::log_level level)
	{
		if (env == nullptr)
		{
			return;
		}
		log_internal("[\n", level);
		for (auto& keyval : env->bindings)
		{
			log_internal("\"" + keyval.first + "\": ", level);
			if (keyval.second != nullptr && keyval.second->car == nullptr && keyval.second->proc != nullptr)
			{
				log_internal("<native func>\n", level);
			}
			else 
			{
				log(keyval.second, level);
				log_internal("\n", level);
			}
		}
		log_internal("]\n", level);
		log_env(env->parent, level);
	}

	void log(const slist::funcdef_ptr& f, slist::log_level level, bool in_pair)
	{
		using namespace slist;

		if (level > global_log_level || f == nullptr)
		{
			return;
		}

		log_internal("Func: ", level);
		log_internal(f->name+"\n", level);

		log_internal("Vars: ", level);
		node_ptr var = f->variables;
		while (var != nullptr)
		{
            if (var->car != nullptr)
            {
                log_internal(var->car->value + " ", level);
            }
            else
            {
                log_internal("nil", level);
            }
			var = var->cdr;
		}
		log_internal("\n", level);

		log_internal("Body: ", level);
		log(f->body, level);
		log_internal("\n", level);

		log_internal("Env:  ", level);
		log_env(f->env, level);
	}

	void log_internal(const std::string& str, slist::log_level level)
	{
		using namespace slist;
		std::ostream *out = nullptr;
		if (level == log_level::always || level == log_level::trace)
		{
			out = &std::cout;
		}
		else 
		{
			out = &std::cerr;
		}
        *out << str << std::flush;
	}
}