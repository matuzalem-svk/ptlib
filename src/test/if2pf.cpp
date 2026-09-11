#include <iostream>
#include "ptlib.h"

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        // todo print help
        std::cout << "usage: if2pf \"infix_expression\" [\"infix_expression\"]*" << std::endl;
        return EXIT_FAILURE;
    }

    ptlib::Parser parser;

    auto expr_ = parser.AddNonterminal("expr_", true);
    auto term_ = parser.AddNonterminal("term_");
    auto factor_ = parser.AddNonterminal("factor_");
    auto t_plus_ = parser.AddTerminal("t_plus", "\\+");
    auto t_times_ = parser.AddTerminal("t_times", "\\*");
    auto t_lpar_ = parser.AddTerminal("t_lpar", "\\(");
    auto t_rpar_ = parser.AddTerminal("t_rpar", "\\)");
    auto t_id_ = parser.AddTerminal("t_id", "[A-Za-z]+");

    parser[expr_] = { expr_, t_plus_, term_ } >> [](auto*, const auto&) { std::cout << "+ " << std::flush; };
    parser[expr_] = { term_ };
    parser[term_] = { term_, t_times_, factor_ } >> [](auto*, const auto&) { std::cout << "* " << std::flush; };
    parser[term_] = { factor_ };
    parser[factor_] = { t_lpar_, expr_, t_rpar_ };
    parser[factor_] = { t_id_ } >> [](auto* head, const auto& body) { std::cout << body.symbols[0]->value << " " << std::flush; };

    for (size_t i = 1; i < argc; ++i)
    {
        if (!parser.Parse(argv[i]))
        {
            return EXIT_FAILURE;
        }

        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}