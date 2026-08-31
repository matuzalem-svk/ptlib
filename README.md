# ptlib - parsing and traslation library
**Author:** Matúš Sabol

## Implementation intent
Efficient generation of LR parsers, translators and lexers from EBNF-like grammar definition.
Author's exercise in formal language theory, modern C++ usage and compile-time computation, project (self-)management.

## Usage intent
Author's exercise in formal language theory application.
For any actual production use, please strongly consider [boost::spirit](https://github.com/boostorg/spirit) or [Spirit.X4](https://github.com/boostorg/spirit_x4) instead.

## Source litarature
Aho, A. V.; Lam, M. S.; Sethi, R.; Ullman, J. D.: *Compilers: Principles, Techniques, and Tools, Second Edition*, Pearson Education Limited, 2014, ISBN 972-1-292-02434-9

## State of the project
### Current state
- naive API for specifying grammars powering automatic parser generation
- runtime SLR parser generation
- placeholder `std::regex`-based lexer
- naive testing "playground"
### Things to be done
#### Mandatory
- tidied-up architecture
- operator overloads for ptlib types for simplifying logging and manipulation
- standalone symbol table shared across lexer and parser
- parsing table rule collision detection during parser construction
    - *reason:* the user needs to know they specified a non-LR(1) grammar
- runtime LALR parser generation
    - *reason:* more powerful parser than SLR with similar memory footprint
- custom lexer generation
    - *reason:* `std::regex` is basically abandonware at this point lacking integration with modern C++ features, using another library defeats the purpose of ptlib
    - should be able to leverage modern C++ features such as `std::string_view` for efficient string manipulation
- semantic action binding to production rules
- grammar symbol operator overloads to be able to write grammars in EBNF-like form (see `boost::spirit`)
#### Optional, but nice to have
- proper testing framework, e.g. gTest + unit test suite
- compile-time lexer/parser generation
    - *reason:* all the necessary information for generating both parser and lexer is known at compile-time, therefore it may be possible to have necessary control tables generated at compile time with template and constexpr metaprogramming using C++23
    - this will slow compilation times in projects, however this should be mitigated by using compile caches in actual production
    - the actual performance gain is questionable since the generation is done just once, but it's still an entertaining exercise in metaprogramming