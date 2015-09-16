cmake_minimum_required(VERSION 3.3)
add_executable (slist 
				slist_types.cpp
				slist_context.cpp
				slist_eval.cpp
				slist_parser.cpp
				slist_native.cpp
				main.cpp)
target_compile_features(slist PRIVATE cxx_range_for)