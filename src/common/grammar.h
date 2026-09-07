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

#include "common/symbols.h"
#include "common/symboltable.h"

namespace ptlib::common {

    typedef std::pair<const Symbol*, std::vector<const Symbol*>> Production;

    struct Grammar
    {
    public:
        Grammar()
        : bAugmented(false), startNonterminal(nullptr)
        {
        }

        ~Grammar()
        {
            if (augmentedStartNonterminal)
            {
                delete augmentedStartNonterminal;
                augmentedStartNonterminal = nullptr;
            }
        }

        const Symbol* AddTerminal(const char* name, const char* value)
        {
            const Symbol* t = SymbolTable::GetInstance().InsertSymbol<SymbolTerminal>(name, value);
            symbols.push_back(t);
            return t;
        }

        const Symbol* AddNonterminal(const char* name, bool isStartNonterminal = false)
        {
            const Symbol* nt = SymbolTable::GetInstance().InsertSymbol<SymbolNonterminal>(name, name);
            symbols.push_back(nt);

            if (isStartNonterminal && !startNonterminal)
            {
                startNonterminal = nt;
            }

            return nt;
        }

        void AddProduction(const Symbol* head, std::vector<const Symbol*> tail)
        {
            Production p(head, tail);
            productions.push_back(p);
        }

        void AugmentGrammar()
        {
            if (bAugmented) return;

            // TODO ensure that the starting nonterminal is actually unique
            static const char* augNtName = "__ptlib_augmented_start_nonterminal__";
            const SymbolNonterminal* nt = dynamic_cast<const SymbolNonterminal*>(SymbolTable::GetInstance().InsertSymbol<SymbolNonterminal>(augNtName, augNtName));

            Production p(nt, { startNonterminal });
            productions.insert(productions.begin(), p);

            augmentedStartNonterminal = nt;

            bAugmented = true;
        }

        std::vector<const Symbol*> symbols;
        std::vector<Production> productions;
        const Symbol* augmentedStartNonterminal;
        const Symbol* startNonterminal;
    private:
        bool bAugmented;
    }; // struct Grammar

} // namespace ptlib::common