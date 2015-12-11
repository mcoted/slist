cmake_minimum_required(VERSION 3.3)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_library(
	slistlib
	slist_types.cpp
	slist_context.cpp
	slist_eval.cpp
	slist_parser.cpp
	slist_native.cpp
	slist_log.cpp
)

set_target_properties(slistlib PROPERTIES OUTPUT_NAME "slist" DEBUG_POSTFIX "d")