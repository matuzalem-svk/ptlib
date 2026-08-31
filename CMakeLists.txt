cmake_minimum_required(VERSION 4.4.2)
project(ptlib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(ptlib)
set_target_properties(ptlib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(ptlib 
    PUBLIC
        src
)
target_sources(ptlib
    PUBLIC
        src/common/types.h
        src/common/utility.h
        src/lexer/regexlexer.h
        src/parser/impl/slr.h
        src/parser/parser.h
        src/ptlib.h
)

add_executable(ptlibtest)
target_sources(ptlibtest PRIVATE src/test/test.cpp)
target_link_libraries(ptlibtest ptlib)