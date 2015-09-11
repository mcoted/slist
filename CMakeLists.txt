add_executable (slist slist_context.cpp slist_eval.cpp slist_parser.cpp main.cpp)
target_compile_features(slist PRIVATE cxx_range_for)