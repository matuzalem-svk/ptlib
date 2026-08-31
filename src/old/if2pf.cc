// Copyright 2019 Matúš Sabol
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT 
// OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "translator.hh"
#include <cstdio>

/** LR(1) production rules for infix */
const char* if2pf_productions_string = 
"S = E\n\
E = E + T\n\
E = T\n\
T = T * F\n\
T = F\n\
F = ( E )\n\
F = $id";

/** translations for generating postfix */
const char* if2pf_translations_string = 
"\n\
+\n\
\n\
*\n\
\n\
\n\
$id";

/** LR(1) parsing table for infix */
const char* if2pf_table_string = 
"( ) $id + * $\n\
S E T F\n\
s4 x s5 x x x\n\
x 1 2 3\n\
x x x s6 x a\n\
x x x x\n\
x x x r3 s7 r3\n\
x x x x\n\
x x x r5 r5 r5\n\
x x x x\n\
s11 x s12 x x x\n\
x 8 9 10\n\
x x x r7 r7 r7\n\
x x x x\n\
s4 x s5 x x x\n\
x x 13 3\n\
s4 x s5 x x x\n\
x x x 14\n\
x s15 x s16 x x\n\
x x x x\n\
x r3 x r3 s17 x\n\
x x x x\n\
x r5 x r5 r5 x\n\
x x x x\n\
s11 x s12 x x x\n\
x 18 9 10\n\
x r7 x r7 r7 x\n\
x x x x\n\
x x x r2 s7 r2\n\
x x x x\n\
x x x r4 r4 r4\n\
x x x x\n\
x x x r6 r6 r6\n\
x x x x\n\
s11 x s12 x x x\n\
x x 19 10\n\
s11 x s12 x x x\n\
x x x 20\n\
x s21 x s16 x x\n\
x x x x\n\
x r2 x r2 s17 x\n\
x x x x\n\
x r4 x r4 r4 x\n\
x x x x\n\
x r6 x r6 r6 x\n\
x x x x";

//////////////////
// DECLARATIONS //
////////////////////////////////////////////////////////////////////
void print_help();

//////////
// MAIN //
////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("ERROR: too few arguments - 1 required\n");
        print_help();
        return 1;
    }

    ptlib::lexer::string_reader lex;
    lex.add_regex_ignore("[ ]+");
    lex.add_regex_use("[a-zA-Z0-9]+", "$id");
    lex.add_regex_use("\\+", "+");
    lex.add_regex_use("\\*", "*");
    lex.add_regex_use("\\(", "(");
    lex.add_regex_use("\\)", ")");
    lex.add_regex_use("$", "$");    // end of input

    ptlib::parser::lr1 lr_t;
    lr_t.set_reader(lex);
    lr_t.load_parsing_table(if2pf_table_string);
    lr_t.load_production_rules(if2pf_productions_string);
    ptlib::translator::lr1_translator iftpf(lr_t);
    iftpf.load_translation_rules(if2pf_translations_string);

    std::string translation_out;
    if(iftpf.translate(argv[1], &translation_out))
    {
        printf("%s\n", translation_out.c_str());
    }
    else
    {
        printf("ERROR: parsing failed\n");
    }

    return 0;
}

/////////////////
// DEFINITIONS //
////////////////////////////////////////////////////////////////////
void print_help()
{
    printf("usage: ./if2pf infix_expr\n");
}