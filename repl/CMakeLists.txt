cmake_minimum_required(VERSION 3.3)

include_directories(${PROJECT_SOURCE_DIR}/include/)

add_executable(repl repl.cpp)
target_link_libraries(repl slist)
