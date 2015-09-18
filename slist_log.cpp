#include "slist_log.h"
#include <iostream>

namespace
{
	slist::log_level global_log_level = slist::log_level::warning;

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
		*out << str;
	}
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

	void output(const std::string& str)
	{
		log(str, log_level::always);
	}

	void output(const node_ptr& n)
	{
		log(n, log_level::always);
	}

	void output(const funcdef_ptr& func)
	{
		log(func, log_level::always);
	}

	void log(const std::string& str, log_level level)
	{
		if (level > global_log_level)
		{
			return;
		}

		log_internal(str, level);
	}

	void log(const node_ptr& n, log_level level)
	{
		if (level > global_log_level)
		{
			return;
		}

		if (n == nullptr)
		{
			log_internal("<null>\n", level);
			return;
		}

		switch (n->type)
		{
			case slist::node_type::empty:
			break;
			case slist::node_type::list:
				if (n->children.size() == 0)
				{
					log_internal("()", level);
				}
				else 
				{
					log_internal("(", level);
					bool first = true;
					for (const slist::node_ptr& child : n->children)
					{
						if (!first)
						{
							log_internal(" ", level);
						}
						log(child, level);
						first = false;
					}
					log_internal(")", level);
				}
			break;
			default:
				log_internal(n->data, level);
			break;
		}

		log_internal("\n", level);
	}

	void log(const funcdef_ptr& func, log_level level)
	{
		if (level > global_log_level)
		{
			return;
		}

		log_internal("Func: ", level);
		log_internal(func->name+"\n", level);

		log_internal("Args: ", level);
		for (auto& arg : func->args)
		{
			log_internal(arg + " ", level);
		}
		if (func->variadic)
		{
			log_internal(" (variadic)", level);
		}
		log_internal("\n", level);

		log_internal("Body: ", level);
		log(func->body, level);
		log_internal("\n", level);
	}
}