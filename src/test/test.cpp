#include "ptlib.h"

int main()
{
    ptlib::parser::Grammar grammar;
    auto expr_ = grammar.AddNonterminal("expr_", true);
    auto term_ = grammar.AddNonterminal("term_");
    auto factor_ = grammar.AddNonterminal("factor_");
    auto t_plus_ = grammar.AddTerminal("t_plus", "\\+");
    auto t_times_ = grammar.AddTerminal("t_times", "\\*");
    auto t_lpar_ = grammar.AddTerminal("t_lpar", "\\(");
    auto t_rpar_ = grammar.AddTerminal("t_rpar", "\\)");
    auto t_id_ = grammar.AddTerminal("t_id", "[A-Za-z]+");

    grammar.AddProduction( expr_, { expr_, t_plus_, term_ } );
    grammar.AddProduction( expr_, { term_ } );
    grammar.AddProduction( term_, { term_, t_times_, factor_ } );
    grammar.AddProduction( term_, { factor_ } );
    grammar.AddProduction( factor_, { t_lpar_, expr_, t_rpar_ } );
    grammar.AddProduction( factor_, { t_id_ } );

    ptlib::Parser parser(grammar);
    parser.Parse("a*(bb+ccc)");

    return EXIT_SUCCESS;
}