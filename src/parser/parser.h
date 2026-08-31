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

#include "parser/impl/slr.h"
#include "lexer/regexlexer.h"

#pragma once

namespace ptlib 
{
using namespace parser;
using namespace lexer;

class Parser
{
public:
    Parser(const ptlib::parser::Grammar& inGrammar)
    : grammar(inGrammar)
    {
        for (const auto* s : grammar.symbols)
        {
            if (!s->IsTerminal()) continue;

            lexer.AddTerminalRegex(s->name, s->value);
        }

        grammar.AugmentGrammar();
        parser = ptlib::parser::SLRParser(grammar);
    }

    bool Parse(const char* input)
    {
        lexer.SetInput(input);
        parser.ResetParse();

        const ptlib::common::Symbol* token = lexer.GetNextToken();
        ptlib::parser::ParseStepResult stepResult;

        do
        {
            stepResult = parser.StepParse(grammar, token);
            if (stepResult.requireNextToken)
            {
                token = lexer.GetNextToken();
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
    ptlib::common::Grammar grammar;
    ptlib::lexer::RegexLexer lexer;
    ptlib::parser::SLRParser parser;
}; // class LexerParser

} // namespace ptlib