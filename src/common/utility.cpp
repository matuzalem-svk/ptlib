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

#include "common/utility.h"
#include <iostream>

namespace ptlib { namespace utility {

void printGrammarProductions(const common::Grammar& grammar)
{
    for (const auto& p : grammar.productions)
    {
        std::cout << p.first->value << " -> ";

        for (const auto& s : p.second)
        {
            std::cout << s->value << "(" << (s->IsTerminal()?"T":"NT") << ") ";
        }

        std::cout << std::endl;
    }
}

void printLRItemSet(const common::Grammar& grammar, const common::LRItemSet& itemSet)
{
    for (const auto& item : itemSet)
    {
        ptlib::common::Production p = grammar.productions[item.first];

        std::cout << p.first->value << " -> ";

        bool dotPrinted = false;
        for (int i = 0; i < p.second.size(); ++i)
        {
            if (i == item.second)
            {
                std::cout << ". ";
                dotPrinted = true;
            }

            std::cout << p.second[i]->value << "(" << (p.second[i]->IsTerminal()?"T":"NT") << ") ";
        }

        if (!dotPrinted)
        {
            std::cout << ".";
        }

        std::cout << std::endl;
    }
}

} } // namespace ptlib::utility