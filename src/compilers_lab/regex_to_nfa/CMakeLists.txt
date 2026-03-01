cmake_minimum_required(VERSION 3.16)
project(regex_to_nfa C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(regex_lib
    src/nfa.c
    src/shunting_yard.c
    src/simulate.c
)

target_include_directories(regex_lib PUBLIC include)

add_executable(regex_to_nfa
    src/main.c
)

target_link_libraries(regex_to_nfa PRIVATE regex_lib)


add_executable(test_shunting_yard
    tests/test_shunting_yard.c
)

target_link_libraries(test_shunting_yard PRIVATE regex_lib)


add_executable(test_nfa
    tests/test_nfa.c
)

target_link_libraries(test_nfa PRIVATE regex_lib)


add_executable(test_simulate
    tests/test_simulate.c
)

target_link_libraries(test_simulate PRIVATE regex_lib)