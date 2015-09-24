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

	void output(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);
	void outputln(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);

	void log_warning(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);
	void log_warningln(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);

	void log_error(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);
	void log_errorln(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);

	void log_trace(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);
	void log_traceln(const std::string& str, node_ptr n = nullptr, funcdef_ptr f = nullptr);
}

#endif