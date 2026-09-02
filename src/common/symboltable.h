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

namespace ptlib { namespace common {

class SymbolTable
{
// singleton interface
public:
    SymbolTable()
    {
        eofSymbol = InsertSymbol<SymbolEOF>("EOF", "");
        emptySymbol = InsertSymbol<SymbolEmpty>("empty", "");
    }

    SymbolTable(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&&) = delete;

    virtual ~SymbolTable()
    {
        for (const Symbol* s : symbols)
        {
            delete s;
        }
        symbols.clear();
        strings.clear();
    }

    static SymbolTable& GetInstance()
    {
        static SymbolTable instance;
        return instance;
    }

// symbol table interface
public:
    template <class SymbolT>
    const Symbol* InsertSymbol(const char* name, const char* value)
    {
        static_assert(std::is_base_of<Symbol, SymbolT>::value, "InsertSymbol() template class must be a subtype of Symbol!");

        strings.insert(value);
        
        const Symbol* out = _insertSymbolImpl<SymbolT>(name, value);
        if (!symbols.contains(out))
        {
            symbols.insert(out);
        }
        else
        {
            const Symbol* s = *(symbols.find(out));
            delete out;
            out = s;
        }

        return out;
    }

    const std::set<const Symbol*> GetSymbols() const
    {
        return symbols;
    }

    const Symbol* GetEOFSymbol() const
    {
        return eofSymbol;
    }

    const Symbol* GetEmptySymbol() const
    {
        return emptySymbol;
    }

protected:
    template <class SymbolT>
    const Symbol* _insertSymbolImpl(const char* name, const char* value) const
    {
        if constexpr (std::is_same_v<SymbolEOF, SymbolT>)
        {
            return new SymbolEOF();
        }
        else if constexpr (std::is_same_v<SymbolEmpty, SymbolT>)
        {
            return new SymbolEmpty();
        }
        else
        {
            return new SymbolT(name, value);
        }
    }

protected:
    std::set<std::string> strings;
    std::set<const Symbol*> symbols;
    const Symbol* eofSymbol;
    const Symbol* emptySymbol;
}; // class SymbolTable

} } // namespace ptlib::common