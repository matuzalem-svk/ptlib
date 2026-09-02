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

#include "lexer/ilexer.h"
#include "common/types.h"
#include "common/symboltable.h"
#include <map>
#include <regex>
#include <iostream>
#include <iomanip>

namespace ptlib { namespace lexer {

class RegexLexer : public ILexerImpl
{
public:
    RegexLexer()
    : currentSymbol(nullptr)
    {
    }

    ~RegexLexer()
    {
        symbolRegexList.clear();
    }

    virtual bool Initialize(const common::Grammar& inGrammar) override
    {
        for (const auto* s : inGrammar.symbols)
        {
            if (!s->IsTerminal()) continue;

            symbolRegexList.push_back({s->name, std::regex(s->value)});
        }

        return true;
    }

    virtual void SetInput(std::string inputString) override
    {
        input = std::move(inputString);
        inputPos = 0;

        std::cout << "lexer input: " << std::quoted(input) << std::endl;
    }

    virtual const common::Symbol* GetToken() const override
    {
        return currentSymbol;
    }

    virtual void NextToken() override
    {
        buffer.clear();

        bool bBeforeMatching = false, bCurrentMatching = false;
        std::map<const char*, bool> matchMap, previousMatchMap;

        size_t bufferViewSize = 1;
        while (1)
        {
            bBeforeMatching = bCurrentMatching;
            bCurrentMatching = false;

            if (inputPos == input.size())
            {
                currentSymbol = common::SymbolTable::GetInstance().GetEOFSymbol();
                return;
            }

            if (inputPos + bufferViewSize > input.size())
            {
                buffer = std::string(input.data() + inputPos);
                goto found_lexeme;
            }

            buffer = std::string(input.data() + inputPos, bufferViewSize);

            std::cout << "lexer buffer (" << inputPos << ", " << bufferViewSize << "): " << std::quoted(buffer) << std::endl;
            std::cout << buffer.data() << std::endl;

            for (auto [tokenName, tokenRegex] : symbolRegexList)
            {
                bool match = std::regex_match(buffer.data(), tokenRegex);
                bCurrentMatching |= match;
                matchMap[tokenName] = match;
            }

            if (bCurrentMatching)
            {
                ++bufferViewSize;
                previousMatchMap = matchMap;
                continue;
            }

            if (!bCurrentMatching && bBeforeMatching)
            {
                buffer = buffer.substr(0, buffer.size()-1);
found_lexeme:
                inputPos += bufferViewSize-1;
                std::cout << "found lexeme " << std::quoted(buffer) << std::endl;
                break;
            }
        }

        const char* tokenName = nullptr;
        for (auto [tokName, bMatch] : previousMatchMap)
        {
            if (bMatch) 
            {
                tokenName = tokName;
                break;
            }
        }

        if (tokenName == nullptr)
        {
            std::cout << "TOKEN NAME IS NULLPTR!!!" << std::endl;
        }

        std::cout << "lexeme(" << std::quoted(tokenName) << ", " << std::quoted(buffer.data()) << ")" << std::endl;

        // TODO really need to make this work with string views, will require either a custom regex implementation or some wrapper around std::regex
        const common::Symbol* out = common::SymbolTable::GetInstance().InsertSymbol<common::SymbolTerminal>(tokenName, buffer.data());
        currentSymbol = out;

        return;
    }

private:
    std::vector<std::pair<const char*, std::regex>> symbolRegexList;

    std::string input;
    size_t inputPos;
    std::string buffer;
    const common::Symbol* currentSymbol;
}; // class RegexLexer


} } // namespace ptlib::lexer