cmake_minimum_required(VERSION 3.10)
project(Kore C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(coylor
    src/main.c
    src/lexer/lexer.c
    src/parser/parser.c
    src/ast/ast.c
    src/interpreter/interpreter.c
)