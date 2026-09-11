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
#include <iostream>
#include <functional>

namespace ptlib::common {

    struct SymbolString
    {
        SymbolString() = default;

        SymbolString(std::initializer_list<const Symbol*> ilist)
        : symbols(ilist)
        {}

        SymbolString& operator>>(const std::function<void(Symbol*, const SymbolString&)>& callback)
        {
            reductionCallback = std::move(callback);
            return *this;
        }

        void push_back(const Symbol* symbol)
        {
            symbols.push_back(symbol);
        }

        std::vector<const Symbol*> symbols;
        std::function<void(Symbol*, const SymbolString&)> reductionCallback;
    };

    typedef std::pair<const SymbolNonterminal*, SymbolString> Production;

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

        SymbolString& operator[](const SymbolNonterminal* head)
        {
            Production p(head, SymbolString{});
            productions.push_back(p);
            return productions.back().second;
        }

        const Production& operator[](size_t idx) const
        {
            return productions[idx];
        }

        void AugmentGrammar()
        {
            if (bAugmented) return;

            // TODO ensure that the starting nonterminal is actually unique
            static const char* augNtName = "__ptlib_augmented_start_nonterminal__";
            const SymbolNonterminal* nt = dynamic_cast<const SymbolNonterminal*>(SymbolTable::GetInstance().InsertSymbol<SymbolNonterminal>(augNtName, augNtName));

            Production p(nt, SymbolString{ startNonterminal });
            productions.insert(productions.begin(), p);

            augmentedStartNonterminal = nt;

            bAugmented = true;
        }

        friend std::ostream& operator<<(std::ostream& os, const Grammar& g)
        {
#ifdef PTLIB_VERBOSE_LOGGING
            for (const auto& p : g.productions)
            {
                os << *p.first << " -> ";

                for (const auto& s : p.second.symbols)
                {
                    os << *s << " ";
                }

                os << std::endl;
            }
#endif

            return os;
        }

        std::vector<const Symbol*> symbols;
        std::vector<Production> productions;
        const Symbol* augmentedStartNonterminal;
        const Symbol* startNonterminal;
    private:
        bool bAugmented;
    }; // struct Grammar

} // namespace ptlib::common