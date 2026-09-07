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

#include <string>
#include <iostream>

namespace ptlib::common {

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

        constexpr virtual bool IsTerminal() const { return false; }
        constexpr virtual bool IsNonterminal() const { return false; }
        constexpr virtual bool IsEOF() const { return false; }
        constexpr virtual bool IsEmpty() const { return false; }

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

        static constexpr const char* _SerializePrefix() { return "<undefined>"; }
        virtual constexpr const char* _SymbolTypeString() const { return Symbol::_SerializePrefix(); }

        template<class SymbolT>
        static std::string Serialize(const char* name, const char* value)
        {
            std::string out;

            out += SymbolT::_SerializePrefix();
            out += name;
            out += value;

            return out;
        }

        friend std::ostream& operator<<(std::ostream& os, const Symbol& s)
        {
#ifdef PTLIB_VERBOSE_LOGGING
            os << "[" << s._SymbolTypeString() << "(" << s.name << "): \"" << s.value << "\"]";
#endif
            return os;
        }

        const char* name;
        const char* value;
    }; // struct Symbol

    struct SymbolEOF : public Symbol
    {
        SymbolEOF()
        : Symbol("EOF", "")
        {}

        constexpr bool IsEOF() const override { return true; }

        static constexpr const char* _SerializePrefix() { return "<EOF>"; }
        constexpr const char* _SymbolTypeString() const override { return SymbolEOF::_SerializePrefix(); }
    }; // struct SymbolEOF

    struct SymbolEmpty : public Symbol
    {
        SymbolEmpty()
        : Symbol("empty", "")
        {}

        constexpr bool IsEmpty() const override { return true; }

        static constexpr const char* _SerializePrefix() { return "<EMPTY>"; }
        constexpr const char* _SymbolTypeString() const override { return SymbolEmpty::_SerializePrefix(); }
    }; // struct SymbolEmpty

    struct SymbolTerminal : public Symbol
    {
        SymbolTerminal(const char* inTermName, const char* inTermValue)
        : Symbol(inTermName, inTermValue)
        {}

        ~SymbolTerminal()
        {}

        constexpr bool IsTerminal() const override { return true; }

        static constexpr const char* _SerializePrefix() { return "<T>"; }
        constexpr const char* _SymbolTypeString() const override { return SymbolTerminal::_SerializePrefix(); }

    }; // struct SymbolTerminal

    struct SymbolNonterminal : public Symbol
    {
        SymbolNonterminal(const char* inName)
        : Symbol(inName, inName)
        {}

        ~SymbolNonterminal()
        {}

        constexpr bool IsNonterminal() const override { return true; }

        static constexpr const char* _SerializePrefix() { return "<NT>"; }
        constexpr const char* _SymbolTypeString() const override { return SymbolNonterminal::_SerializePrefix(); }
    }; // struct SymbolNonterminal

} // namespace ptlib::common