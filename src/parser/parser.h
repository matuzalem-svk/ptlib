// Copyright 2026 Matus Sabol
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

#pragma once

#include "parser/iparser.h"
#include "parser/impl/slr.h"
#include "lexer/ilexer.h"
#include "lexer/impl/regexlexer.h"

namespace ptlib 
{
using namespace common;
using namespace parser;
using namespace lexer;

template <class ParserImplClassT = SLRParser, class LexerImplClassT = RegexLexer>
class Parser
{
public:
    Parser(const Grammar& inGrammar)
    : grammar(inGrammar), bParserInitialized(false)
    {
        static_assert(std::is_base_of<ILexerImpl, LexerImplClassT>::value, "Lexer class does not implement common ILexer interface.");
        static_assert(std::is_base_of<IParserImpl, ParserImplClassT>::value, "Parser class does not implement common IParser interface.");

        lexer.Initialize(grammar);

        grammar.AugmentGrammar();
        bParserInitialized = parser.Initialize(grammar);
    }

    bool Parse(const char* input)
    {
        if (!bParserInitialized)
        {
            std::cout << "PARSER INITIALIZATION FAILED" << std::endl;
            return false;
        }

        lexer.SetInput(input);
        parser.ResetParse();

        lexer.NextToken();
        ParseStepResult stepResult;

        do
        {
            const Symbol* token = lexer.GetToken();
            stepResult = parser.StepParse(token);
            if (stepResult.requireNextToken)
            {
                lexer.NextToken();
            }

            if (stepResult.actionSuccess && stepResult.parseSuccess)
            {
                std::cout << "PARSE SUCCESSFUL" << std::endl;
                break;
            }
        }
        while (stepResult.actionSuccess);

        return stepResult.parseSuccess;
    }

protected:
    Grammar grammar;
    LexerImplClassT lexer;
    ParserImplClassT parser;
    bool bParserInitialized;
}; // class Parser

} // namespace ptlib