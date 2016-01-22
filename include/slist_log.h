#ifndef SLIST_LOG_H
#define SLIST_LOG_H

#include "slist_types.h"

#ifdef DEBUG
#define LOG_TRACE(STR) log_trace_slow(STR)
#define LOG_TRACE2(STR, ARG0) log_trace_slow(STR, ARG0)
#define LOG_TRACELN(STR) LOG_TRACELN_slow(STR)
#define LOG_TRACELN2(STR, ARG0) LOG_TRACELN_slow(STR, ARG0)
#else
#define LOG_TRACE(STR) do {} while (0)
#define LOG_TRACE2(STR, ARG0) do {} while (0)
#define LOG_TRACELN(STR) do {} while (0)
#define LOG_TRACELN2(STR, ARG0) do {} while (0)
#endif

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

	void output(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr, bool from_print = false);
	void outputln(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr, bool from_print = false);

	void log_warning(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);
	void log_warningln(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);

	void log_error(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);
	void log_errorln(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);

	void log_trace_slow(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);
	void log_traceln_slow(const std::string& str = "", node_ptr n = nullptr, procedure_ptr f = nullptr);
}

#endif