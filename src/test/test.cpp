#define PTLIB_VERBOSE_LOGGING
#include "ptlib.h"

int main()
{
    /*ptlib::parser::Grammar grammar;
    auto expr_ = grammar.AddNonterminal("expr_", true);
    auto term_ = grammar.AddNonterminal("term_");
    auto factor_ = grammar.AddNonterminal("factor_");
    auto t_plus_ = grammar.AddTerminal("t_plus", "\\+");
    auto t_times_ = grammar.AddTerminal("t_times", "\\*");
    auto t_lpar_ = grammar.AddTerminal("t_lpar", "\\(");
    auto t_rpar_ = grammar.AddTerminal("t_rpar", "\\)");
    auto t_id_ = grammar.AddTerminal("t_id", "[A-Za-z]+");

    grammar[expr_] = { expr_, t_plus_, term_ };
    grammar[expr_] = { term_ };
    grammar[term_] = { term_, t_times_, factor_ };
    grammar[term_] = { factor_ };
    grammar[factor_] = { t_lpar_, expr_, t_rpar_ };
    grammar[factor_] = { t_id_ };

    ptlib::Parser parser(grammar);*/

    ptlib::Parser parser;

    auto expr_ = parser.AddNonterminal("expr_", true);
    auto term_ = parser.AddNonterminal("term_");
    auto factor_ = parser.AddNonterminal("factor_");
    auto t_plus_ = parser.AddTerminal("t_plus", "\\+");
    auto t_times_ = parser.AddTerminal("t_times", "\\*");
    auto t_lpar_ = parser.AddTerminal("t_lpar", "\\(");
    auto t_rpar_ = parser.AddTerminal("t_rpar", "\\)");
    auto t_id_ = parser.AddTerminal("t_id", "[A-Za-z]+");

    parser[expr_] = { expr_, t_plus_, term_ };
    parser[expr_] = { term_ };
    parser[term_] = { term_, t_times_, factor_ };
    parser[term_] = { factor_ };
    parser[factor_] = { t_lpar_, expr_, t_rpar_ };
    parser[factor_] = { t_id_ };

    parser.Parse("a+a*(bb+ccc)");

    return EXIT_SUCCESS;
}