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

#include "common/types.h"
#include <set>
#include <map>
#include <regex>
#include <string_view>
#include <iostream>
#include <iomanip>

namespace ptlib { namespace lexer {

class RegexLexer
{
public:
    RegexLexer()
    {
        eofSymbol = new common::SymbolEOF();
    }

    ~RegexLexer()
    {
        for (const common::Symbol* s : symbolTable)
        {
            delete s;
        }

        delete eofSymbol;

        symbolTable.clear();
        symbolRegexList.clear();
    }

    void AddTerminalRegex(const char* inName, const char* inRegex)
    {
        symbolRegexList.push_back({inName, std::regex(inRegex)});
    }

    void SetInput(const char* inputString)
    {
        input = inputString;
        inputPos = 0;

        std::cout << "lexer input: " << std::quoted(input) << std::endl;
    }

    const common::Symbol* GetNextToken()
    {
        buffer.clear();

        bool bBeforeMatching = false, bCurrentMatching = false;
        std::map<const char*, bool> matchMap, previousMatchMap;

        size_t bufferViewSize = 1;
        while (1)
        {
            bBeforeMatching = bCurrentMatching;
            bCurrentMatching = false;

            if (inputPos == strlen(input))
            {
                return eofSymbol;
            }

            if (inputPos + bufferViewSize > strlen(input))
            {
                buffer = std::string(input + inputPos);
                goto found_lexeme;
            }

            buffer = std::string(input + inputPos, bufferViewSize);

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
        // Symbol value will be garbage by the time next symbol is created
        const common::Symbol* out = new common::SymbolTerminal(tokenName, buffer.data());
        if (!symbolTable.contains(out))
        {
            symbolTable.insert(out);
        }
        else
        {
            const common::Symbol* s = *(symbolTable.find(out));
            delete out;
            out = s;
        }

        return out;
    }

private:
    std::vector<std::pair<const char*, std::regex>> symbolRegexList;

    const char* input;
    size_t inputPos;
    std::string buffer;
    std::set<const common::Symbol*> symbolTable;
    common::Symbol* eofSymbol;
}; // class RegexLexer


} } // namespace ptlib::lexer