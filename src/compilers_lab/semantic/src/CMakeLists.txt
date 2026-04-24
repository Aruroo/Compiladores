cmake_minimum_required(VERSION 3.10)
project(carta)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

FLEX_TARGET(Scanner scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_executable(carta
    ${FLEX_Scanner_OUTPUTS}
    ${BISON_Parser_OUTPUTS}
    ast.cpp
    main.cpp
)