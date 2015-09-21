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

	void output(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		log(str, log_level::always);
		log(n, log_level::always);
		log(f, log_level::always);
	}

	void outputln(const std::string& str, node_ptr n, funcdef_ptr f)
	{
		output(str, n, f);
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
		if (level > global_log_level || n == nullptr)
		{
			return;
		}

		if (n == nullptr)
		{
			if (global_log_level >= log_level::trace)
			{
				log_internal("<null>\n", level);
			}
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
	}

	void log(const funcdef_ptr& f, log_level level)
	{
		if (level > global_log_level || f == nullptr)
		{
			return;
		}

		log_internal("Func: ", level);
		log_internal(f->name+"\n", level);

		log_internal("Args: ", level);
		for (auto& arg : f->args)
		{
			log_internal(arg + " ", level);
		}
		if (f->variadic)
		{
			log_internal(" (variadic)", level);
		}
		log_internal("\n", level);

		log_internal("Body: ", level);
		log(f->body, level);
		log_internal("\n", level);
	}
}