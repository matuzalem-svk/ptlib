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
        
        const Symbol* out = new SymbolT(name, value);
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
        static SymbolEOF eofSymbol;
        return &eofSymbol;
        static SymbolEOF eofSymbol;
        return &eofSymbol;
    }

    const Symbol* GetEmptySymbol() const
    {
        static SymbolEmpty emptySymbol;
        return &emptySymbol;
    }

protected:
    std::set<std::string> strings;
    std::set<const Symbol*> symbols;
}; // class SymbolTable

} } // namespace ptlib::common