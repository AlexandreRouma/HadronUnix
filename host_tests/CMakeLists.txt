cmake_minimum_required(VERSION 3.13)
project(htest)

file(GLOB_RECURSE SRC "src/*.c")

add_executable(htest ${SRC})

target_include_directories(htest PRIVATE "src")
target_compile_options(htest PRIVATE -Wall -Wextra -g -Og -fsanitize=address)
target_link_libraries(htest PRIVATE asan)