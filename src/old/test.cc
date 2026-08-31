#include "parser.h"

int main()
{
    simple_parser::rule<std::string> E("expr");
    simple_parser::rule<std::string> T("term");
    simple_parser::rule<std::string> F("factor");

    E = (E >> "+" >> T)[E >> T >> "+"];
    E |= (T)[T];
    T = (T >> "*" >> F)[T >> F >> "*"];
    T |= F[F];
    F = ((std::string)"a");
    F |= ((std::string)"(" >> E >> ")")[E];

    E();
    T();
    F();

    std::cout << "=================================\n" << std::endl;

    simple_parser::rule<std::string> PROGRAM("program");
    simple_parser::rule<std::string> EXPRLIST("exprlist");
    simple_parser::rule<std::string> EXPR("expr");
    simple_parser::rule<std::string> FUNC("func");
    simple_parser::rule<std::string> FUNCID("funcid");
    simple_parser::rule<std::string> VARLIST("varlist");
    simple_parser::rule<std::string> VAR("var");

    PROGRAM   = EXPRLIST;
    EXPRLIST  = EXPR             >> ";" >> EXPRLIST;
    EXPRLIST |= EXPR             >> ";";
    EXPR      = FUNC;
    FUNC      = FUNCID           >> "(" >> VARLIST >> ")";
    FUNCID    = "f";
    VARLIST   = VAR              >> VARLIST;
    VARLIST  |= (std::string)"," >> VARLIST;
    VARLIST  |= VAR;
    VAR       = "a";

    PROGRAM();
    EXPRLIST();
    EXPR();
    FUNC();
    VARLIST();
    VAR();

    return 0;
}