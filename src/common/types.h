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

#include <cstdint>
#include <vector>
#include <set>
#include <memory>
#include <iostream>

namespace ptlib { namespace common {

    struct Symbol
    {
        Symbol(const char* inName, const char* inValue)
        : name(inName), value(inValue)
        {}

        Symbol(const Symbol& inSymbol)
        : name(inSymbol.name), value(inSymbol.value)
        {}

        virtual ~Symbol()
        {}

        virtual bool IsTerminal() const
        {
            return false;
        }

        virtual bool IsNonterminal() const
        {
            return false;
        }

        virtual bool IsEOF() const
        {
            return false;
        }

        virtual bool IsEmpty() const
        {
            return false;
        }

        bool TypeEquals(const Symbol* other) const
        {
            return 
               this->IsTerminal() == other->IsTerminal() 
            && this->IsNonterminal() == other->IsNonterminal() 
            && this->IsEOF() == other->IsEOF() 
            && this->IsEmpty() == other->IsEmpty()
            && strcmp(this->name, other->name) == 0;
        }

        bool ValueEquals(const Symbol* other) const
        {
            return strcmp(this->value, other->value) == 0;
        }

        bool operator==(const Symbol& other) const
        {
            return TypeEquals(&other) && ValueEquals(&other);
        }

        const char* name;
        const char* value;
    }; // struct Symbol

    struct SymbolEOF : virtual public Symbol
    {
        SymbolEOF()
        : Symbol("EOF", "")
        {}

        bool IsEOF() const override { return true; }
    }; // struct SymbolEOF

    struct SymbolEmpty : virtual public Symbol
    {
        SymbolEmpty()
        : Symbol("empty", "")
        {}

        bool IsEmpty() const override { return true; }
    }; // struct SymbolEmpty

    struct SymbolTerminal : virtual public Symbol
    {
        SymbolTerminal(const char* inTermName, const char* inTermValue)
        : Symbol(inTermName, inTermValue)
        {}

        ~SymbolTerminal()
        {}

        bool IsTerminal() const override { return true; }

    }; // struct SymbolTerminal

    struct SymbolNonterminal : virtual public Symbol
    {
        SymbolNonterminal(const char* inName)
        : Symbol(inName, inName)
        {}

        ~SymbolNonterminal()
        {}

        bool IsNonterminal() const override { return true; }
    }; // struct SymbolNonterminal

    typedef std::pair<const Symbol*, std::vector<const Symbol*>> Production;

    struct Grammar
    {
    public:
        Grammar()
        : bAugmented(false), startNonterminal(nullptr)
        {
            eofTerminal = new SymbolEOF;
            emptyTerminal = new SymbolEmpty;
        }

        ~Grammar()
        {
            for (Symbol* s : symbols)
            {
                if (s)
                {
                    delete s;
                    s = nullptr;
                }
            }

            delete augmentedStartNonterminal;
            delete eofTerminal;
            delete emptyTerminal;
        }

        Symbol* AddTerminal(const char* name, const char* value)
        {
            SymbolTerminal* t = new SymbolTerminal(name, value);
            symbols.push_back(t);
            return t;
        }

        Symbol* AddNonterminal(const char* name, bool isStartNonterminal = false)
        {
            SymbolNonterminal* nt = new SymbolNonterminal(name);
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
            SymbolNonterminal* nt = new SymbolNonterminal("lr_augmented_start_nonterminal_");
            //symbols.insert(symbols.begin(), nt);

            Production p(nt, { startNonterminal });
            productions.insert(productions.begin(), p);

            augmentedStartNonterminal = nt;

            bAugmented = true;
        }

        std::vector<Symbol*> symbols;
        std::vector<Production> productions;
        Symbol* augmentedStartNonterminal;
        Symbol* startNonterminal;
        Symbol* eofTerminal;
        Symbol* emptyTerminal;
    private:
        bool bAugmented;
    }; // struct Grammar

    typedef size_t LRStateID;
    typedef std::pair<LRStateID, int> LRItem;
    typedef std::vector<LRItem> LRItemSet;
    typedef std::vector<LRItemSet> LRItemSetCollection;

} } // namespace ptlib::common