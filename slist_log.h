#ifndef SLIST_LOG_H
#define SLIST_LOG_H

#include "slist_types.h"

namespace slist
{
	enum class log_level
	{
		always  = 0,
		error   = 1,
		warning = 2,
		trace   = 3
	};

	log_level get_log_level();
	void set_log_level(log_level level);

	void output(const std::string& str);
	void output(const node_ptr& n);
	void output(const funcdef_ptr& func);	

	void log(const std::string& str, log_level level);
	void log(const node_ptr& n, log_level level);
	void log(const funcdef_ptr& func, log_level level);
}

#endif