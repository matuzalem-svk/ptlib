cmake_minimum_required(VERSION 4.4.2)
project(ptlib VERSION 1.0.0 LANGUAGES C CXX)

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
        src/lexer/lexemerepeater.h
        src/lexer/regexlexer.h
        src/parser/ilrparser.h
        src/parser/slr.h
)
target_sources(ptlib
    PRIVATE
        src/common/utility.cpp
        src/parser/ilrparser.cpp
        src/parser/slr.cpp
)


add_executable(ptlibtest)
target_sources(ptlibtest PRIVATE src/test.cpp)
target_link_libraries(ptlibtest ptlib)